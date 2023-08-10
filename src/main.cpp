#include "simulator.h"
#include "argparse.h"

#include <limits>
#include <climits>

int main(int argc, char *argv[])
{
	argparse::ArgumentParser program("Blockchain-Simulator");
	program.add_argument("-n")
		.help("number of peers in the network")
		.default_value((int)40)
		.action([](const std::string &value)
				{ return stoi(value); });

	program.add_argument("-z0")
		.help("fraction of \"slow\" peers in the network")
		.default_value((double)0.5)
		.action([](const std::string &value)
				{ return stod(value); });

	program.add_argument("-z1")
		.help("fraction of \"low CPU\" peers in the network")
		.default_value((double)0.5)
		.action([](const std::string &value)
				{ return stod(value); });

	program.add_argument("-Ttx")
		.help("mean of interarrival time between txns")
		.default_value((double)10.0)
		.action([](const std::string &value)
				{ return stod(value); });

	program.add_argument("-blks")
		.help("max blocks in the simulation")
		.default_value((int)500)
		.action([](const std::string &value)
				{ return stoi(value); });

	program.add_argument("-txns")
		.help("max txns in the simulation")
		.default_value((int)INT_MAX)
		.action([](const std::string &value)
				{ return stoi(value); });

	program.add_argument("-zeta")
		.help("fraction of honest nodes an adversary is connected to")
		.default_value((int)50)
		.action([](const std::string &value)
				{ return stoi(value); });

	program.add_argument("-selfish")
		.help("Adversary node is selfish")
		.default_value((bool)0)
		.implicit_value((bool)1);

	program.add_argument("-stubborn")
		.help("Adversary node is stubborn")
		.default_value((bool)0)
		.implicit_value((bool)1);

	program.add_argument("-hash_fraction")
		.help("fraction of hash power an adversary has")
		.default_value((int)30)
		.action([](const std::string &value)
				{ return stoi(value); });

	program.add_argument("-stime")
		.help("simulation time in secs")
		.default_value((double)std::numeric_limits<double>::max())
		.action([](const std::string &value)
				{ return stod(value); });

	program.add_argument("-I")
		.help("block interarrival time in secs")
		.default_value((double)20.0)
		.action([](const std::string &value)
				{ return stod(value); });

	program.add_argument("-blkchain", "--blockchain_file")
		.help("name of file to write blockchain info to")
		.default_value((std::string) "trace")
		.action([](const std::string &value)
				{ return value; });

	program.add_argument("-graph", "--graph_file")
		.help("name of file to write blockchain edges to")
		.default_value((std::string) "edges")
		.action([](const std::string &value)
				{ return value; });

	program.add_argument("-stats", "--stats_file")
		.help("name of file to write simulation stats to")
		.default_value((std::string) "stats")
		.action([](const std::string &value)
				{ return value; });

	program.add_argument("-data", "--data_file")
		.help("name of file to visualization data to")
		.default_value((std::string) "data")
		.action([](const std::string &value)
				{ return value; });

	program.add_argument("-v", "--visualize")
		.help("produce visualization data")
		.default_value((bool)false)
		.implicit_value((bool)true);

	try
	{
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error &err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}
	int n = program.get<int>("-n");
	double z0 = program.get<double>("-z0");
	double z1 = program.get<double>("-z1");
	double Ttx = program.get<double>("-Ttx");
	int zeta = program.get<int>("-zeta");
	bool selfish_attack = program.get<bool>("-selfish");
	bool stubborn_attack = program.get<bool>("-stubborn");
	int adversary_hash_power = program.get<int>("-hash_fraction");
	int max_blocks = program.get<int>("-blks");
	int max_txns = program.get<int>("-txns");
	simulator s(n, z0, z1, Ttx);
	s.simulation_time = program.get<double>("-stime");
	s.blk_inter_arrival_time = program.get<double>("-I");
	; // 10 min avg time (whole network) generate new block
	s.max_blocks = max_blocks;
	s.max_txns = max_txns;
	s.simulate_selfish_attack = selfish_attack;
	s.simulate_stubborn_attack = stubborn_attack;
	s.adversary_hash_power = adversary_hash_power;
	s.zeta = zeta;
	s.trace_file = program.get<std::string>("-blkchain");
	s.graph_file = program.get<std::string>("-graph");
	s.stats_file = program.get<std::string>("-stats");
	s.visual_file = program.get<std::string>("-data");
	s.visualize = program.get<bool>("-v");
	s.initialize();
	s.start();
}