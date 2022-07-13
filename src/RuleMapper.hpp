#ifndef RULEMAPPER_H
#define RULEMAPPER_H
#include "pch.hpp"
#include "MidiEvent.hpp"
#include "log.hpp"
using namespace std;

class MidiClient;

class RuleMapper {
private:
	static const int sleep_ms;
	const MidiClient* midi_client;
public:
	RuleMapper(const string& fileName, MidiClient* mc);
	int findMatchingRule(const MidiEvent&, int startPos = 0) const;
	void parseString(const string&);
	bool applyRules(MidiEvent& ev);
	MidiEventRule& getRule(int i) {
		return rules[i];
	}
	size_t getSize() const {
		return rules.size();
	}
	string toString() const;

private:

	//time_point prev_moment = the_clock::now();
	MidiEvent prev_count_ev;
	MidiEvent prev_ev;

	int count_on = 0;
	int count_off = 0;

	vector<MidiEventRule> rules;

	void update_count(const MidiEvent& ev);
	void count_and_send(const MidiEvent& ev, int cnt_on);

};

#endif
