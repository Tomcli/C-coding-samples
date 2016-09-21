// $Id: cix.cpp,v 1.4 2016-05-09 16:01:56-07 - - $

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string, cix_command> command_map {
	{"exit", cix_command::EXIT},
	{"help", cix_command::HELP},
	{"ls"  , cix_command::LS  },
	{"get" , cix_command::GET },
	{"rm"  , cix_command::RM  },
	{"put" , cix_command::PUT },
};

void cix_help() {
	static const vector<string> help = {
		"exit         - Exit the program.  Equivalent to EOF.",
		"get filename - Copy remote file to local host.",
		"help         - Print help summary.",
		"ls           - List names of files on remote server.",
		"put filename - Copy local file to remote host.",
		"rm filename  - Remove file from remote server.",
	};
	for (const auto& line : help) cout << line << endl;
}

void cix_ls (client_socket& server) {
	cix_header header;
	header.command = cix_command::LS;
	log << "sending header " << header << endl;
	send_packet (server, &header, sizeof header);
	recv_packet (server, &header, sizeof header);
	log << "received header " << header << endl;
	if (header.command != cix_command::LSOUT) {
		log << "sent LS, server did not return LSOUT" << endl;
		log << "server returned " << header << endl;
	} else {
		char buffer[header.nbytes + 1];
		recv_packet (server, buffer, header.nbytes);
		log << "received " << header.nbytes << " bytes" << endl;
		buffer[header.nbytes] = '\0';
		cout << buffer;
	}
}

void cix_get(client_socket& server, string file) {
	cix_header header;
	header.command = cix_command::GET;
	//copy the file to header.filename
	strcpy(header.filename, file.c_str());
	//sent get header
	log << "sending header " << header << endl;
	send_packet (server, &header, sizeof header);
	recv_packet (server, &header, sizeof header);
	log << "received header " << header << endl;
	if (header.command != cix_command::FILE) {
		log << "sent GET, server did not return FILE" << endl;
		log << "server returned " << header << endl;
	} else {
		char buffer[header.nbytes + 1];
		recv_packet(server, buffer, header.nbytes);
		log << "successfully received file " << file << endl;
		log << "received " << header.nbytes << " bytes" << endl;
		buffer[header.nbytes] = '\0';
		//create an ostream file and write on that file
		ofstream getfile (file, ofstream::out);
		getfile.write(buffer, header.nbytes);
		getfile.close();
	}
}

void cix_rm(client_socket& server, string file) {
	cix_header header;
	header.command = cix_command::RM;
	header.nbytes = 0;
	strcpy(header.filename, file.c_str());
	//sent rm header
	log << "sending header " << header << endl;
	send_packet (server, &header, sizeof header);
	recv_packet (server, &header, sizeof header);
	log << "received header " << header << endl;
	if (header.command != cix_command::ACK) {
		log << "sent RM, server did not return ACK" << endl;
		log << "server returned " << header << endl;
	} else {
		log << "successfully removed file " << file << endl;
	}
}

void cix_put(client_socket& server, string file) {
	cix_header header;
	header.command = cix_command::PUT;
	ifstream putfile (file);
	//using stat to find the file size.
	struct stat filenum;
	stat(file.c_str(), &filenum);
	header.nbytes = filenum.st_size;
	//if the file is not vaild, return
	if (putfile == NULL) {
		return;
	}
	char buffer[header.nbytes +1];
	putfile.read(buffer, header.nbytes);
	strcpy(header.filename, file.c_str());
	//sent put header
	log << "sending header " << header << endl;
	send_packet (server, &header, sizeof header);
	//sent the file as a buffer
	send_packet (server, buffer, sizeof buffer);
	recv_packet (server, &header, sizeof header);
	log << "received header " << header << endl;
	if (header.command != cix_command::ACK) {
		log << "sent PUT, server did not return ACK" << endl;
		log << "server returned " << header << endl;
	} else {
		log << "successfully sent file " << file << endl;
	}
	putfile.close();
}



void usage() {
	cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
	throw cix_exit();
}

int main (int argc, char** argv) {
	log.execname (basename (argv[0]));
	log << "starting" << endl;
	vector<string> args (&argv[1], &argv[argc]);
	if (args.size() > 2) usage();
	string host = get_cix_server_host (args, 0);
	in_port_t port = get_cix_server_port (args, 1);
	log << to_string (hostinfo()) << endl;
	try {
		log << "connecting to " << host << " port " << port << endl;
		client_socket server (host, port);
		log << "connected to " << to_string (server) << endl;
		for (;;) {
			string line;
			getline (cin, line);
			if (cin.eof()) throw cix_exit();
			string command = line.substr(0, line.find_first_of(" "));
			log << "command " << command << endl;
			string argument = "";
			//anything after the command is considered as filenames
			if (line.find_first_of(" ") != string::npos) {
				argument = line.substr(line.find_first_of(" ") + 1 , string::npos);
			}
			log << "argument " << argument << endl;
			const auto& itor = command_map.find (command);
			cix_command cmd = itor == command_map.end()
			                  ? cix_command::ERROR : itor->second;
			switch (cmd) {
			case cix_command::EXIT:
				throw cix_exit();
				break;
			case cix_command::HELP:
				cix_help();
				break;
			case cix_command::LS:
				cix_ls (server);
				break;
			case cix_command::GET:
				cix_get(server, argument);
				break;
			case cix_command::RM:
				cix_rm(server, argument);
				break;
			case cix_command::PUT:
				cix_put(server, argument);
				break;
			default:
				log << line << ": invalid command" << endl;
				break;
			}
		}
	} catch (socket_error& error) {
		log << error.what() << endl;
	} catch (cix_exit& error) {
		log << "caught cix_exit" << endl;
	}
	log << "finishing" << endl;
	return 0;
}

