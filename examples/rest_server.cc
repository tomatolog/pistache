/* 
   Mathieu Stefani, 07 février 2016
   
   Example of a REST endpoint with routing
*/

#include <algorithm>
#include <sstream>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include "rocksdb/db.h"
#include "rocksdb_replicator/rocksdb_replicator.h"
#include <gflags/gflags.h>

DEFINE_int32(thd_count, 2, "worker count");
DEFINE_int32(http_port, 9080, "port of HTTP REST interface");

DEFINE_string(db_path, "/tmp/db/testdb", "path to Rocks DB");
DEFINE_string(db_name, "shard1", "name of Rocks DB shard");
DEFINE_string(replicator_role, "slave,master", "role of DB replicator");
DEFINE_int32(replicator_self_port, 9081, "port of replicator itself");
DEFINE_string(replicator_host, "127.0.0.1", "host of replicator interface");
DEFINE_int32(replicator_port, 9082, "port of replicator interface");
DECLARE_int32(rocksdb_replicator_port);

using namespace std;
using namespace Pistache;

void printCookies(const Http::Request& req) 
{
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) 
	{
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic 
{
	void handleReady(const Rest::Request&, Http::ResponseWriter response) 
	{
		response.send(Http::Code::Ok, "1");
	}
}

class StatsEndpoint
{
public:
    StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    {}

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(thr)
			.flags(Tcp::Options::ReuseAddr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
		setupDB();
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

	int stopped = 0;

private:
    void setupRoutes() 
	{
        using namespace Rest;

        Routes::Post(router, "/record/:name/:value?", Routes::bind(&StatsEndpoint::doPost, this));
        Routes::Get(router, "/value/:name", Routes::bind(&StatsEndpoint::doGet, this));
		Routes::Get(router, "/all", Routes::bind(&StatsEndpoint::getAll, this));
		Routes::Get(router, "/seq", Routes::bind(&StatsEndpoint::getSeq, this));

        Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        Routes::Get(router, "/auth", Routes::bind(&StatsEndpoint::doAuth, this));

        Routes::Post(router, "/stop", Routes::bind(&StatsEndpoint::Exit, this));
        Routes::Get(router, "/stop", Routes::bind(&StatsEndpoint::Exit, this));

    }

    void Exit( const Rest::Request & , Http::ResponseWriter ) 
	{
		stopped = 1;
	}

    void doPost(const Rest::Request& request, Http::ResponseWriter response) 
	{
        auto name = request.param(":name").as<std::string>();

        std::string sVal = "";
        if (request.hasParam(":value")) {
            auto value = request.param(":value");
            sVal = value.as<std::string>();
        }

		std::string sOld;
		db->Get(rocksdb::ReadOptions(), name, &sOld);
		db->Put(rocksdb::WriteOptions(), name, sVal);

		std::stringstream sRes;
		sRes << name << ":" << sVal << "(" << sOld << ")" << endl;
		response.send ( Http::Code::Ok, sRes.str() );
    }

    void doGet(const Rest::Request& request, Http::ResponseWriter response) 
	{
        auto name = request.param(":name").as<std::string>();

		std::string sOld;
		db->Get(rocksdb::ReadOptions(), name, &sOld);

		std::stringstream sRes;
		sRes << name << ":" << sOld << endl;
		response.send ( Http::Code::Ok, sRes.str() );
    }

    void getAll(const Rest::Request& request, Http::ResponseWriter response) 
	{
		std::stringstream sRes;
		rocksdb::Iterator * it = db->NewIterator ( rocksdb::ReadOptions() );
		for ( it->SeekToFirst(); it->Valid(); it->Next() )
		{
			sRes << it->key().ToString() << ": " << it->value().ToString() << endl;
		}
		delete it;

		response.send(Http::Code::Ok, sRes.str() );
    }

    void getSeq(const Rest::Request& request, Http::ResponseWriter response) 
	{
		std::stringstream sSeq;
		sSeq << db->GetLatestSequenceNumber() << endl;
		response.send ( Http::Code::Ok, sSeq.str() );
    }

    void doAuth(const Rest::Request& request, Http::ResponseWriter response) 
	{
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        response.send(Http::Code::Ok);
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;

	void setupDB ()
	{
		rocksdb::Options options;
		options.create_if_missing = true;
		rocksdb::DB * dbRaw;
		rocksdb::Status status = rocksdb::DB::Open ( options, FLAGS_db_path, &dbRaw );
		db.reset( dbRaw );

		folly::SocketAddress addr ( FLAGS_replicator_host, FLAGS_replicator_port );

		replicator::DBRole role = replicator::DBRole::SLAVE;
		if ( FLAGS_replicator_role=="master" )
		{
			role = replicator::DBRole::MASTER;
			FLAGS_rocksdb_replicator_port = FLAGS_replicator_self_port;
			addr = folly::SocketAddress();
		}

		replicator = replicator::RocksDBReplicator::instance();
		replicator->addDB ( FLAGS_db_name, db, role, addr, &replicated_db );
	}

	std::shared_ptr<rocksdb::DB> db;
	replicator::RocksDBReplicator * replicator;
	replicator::RocksDBReplicator::ReplicatedDB * replicated_db;
};

int main(int argc, char *argv[]) 
{
	gflags::ParseCommandLineFlags(&argc, &argv, true);

    Port port(FLAGS_http_port);
    int thr = FLAGS_thd_count;

    Address addr(Ipv4::any(), port);

    cout << "cores = " << hardware_concurrency() << " threads = " << thr << " port = " << port << endl;

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();

	while ( !stats.stopped )
		sleep(1);

    stats.shutdown();
}
