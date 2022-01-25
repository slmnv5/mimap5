#include "pch.hpp"
#include "utils.hpp"

using namespace std;

//==================== utility functions ===================================

vector<string> split_string(const string &s, const string &delimiter) {
	vector<string> tokens;
	auto start = 0U;
	auto stop = s.find(delimiter);
	while (stop != string::npos) {
		tokens.push_back(s.substr(start, stop - start));
		start = stop + delimiter.length();
		stop = s.find(delimiter, start);
	}
	tokens.push_back(s.substr(start, stop));
	return tokens;
}

int replace_all(string &s, const string &del, const string &repl) {
	string::size_type delsz = del.size();
	if (delsz == 0)
		return 0;
	int count = 0;
	string::size_type n = 0;
	while ((n = s.find(del, n)) != string::npos) {
		s.replace(n, delsz, repl);
		n += repl.size();
		count++;
	}
	return count;
}

void remove_spaces(string &s) {
	s = s.substr(0, s.find(";"));
	replace_all(s, " ", "");
	replace_all(s, "\n", "");
	replace_all(s, "\t", "");
}

string exec_command(const string &cmd) {
	char buffer[128];
	std::string result = "";
	FILE *pipe = popen(cmd.c_str(), "r");
	if (!pipe)
		throw std::runtime_error("popen() failed!");
	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += buffer;
		}
	} catch (exception &e) {
		pclose(pipe);
		throw e;
	}
	pclose(pipe);
	return result;
}
