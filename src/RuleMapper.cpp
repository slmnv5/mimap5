#include "pch.hpp"
#include "MidiEvent.hpp"
#include "RuleMapper.hpp"
#include "MidiClient.hpp"
#include "log.hpp"

using namespace std;

const int RuleMapper::sleep_ms = 600;

RuleMapper::RuleMapper(const string &fileName, const MidiClient &mc) :
		midi_client(mc) {
	ifstream f(fileName);
	string s;
	int k = 0;
	while (getline(f, s)) {
		try {
			k++;
			rules.push_back(MidiEventRule(s));
		} catch (MidiAppError &e) {
			LogLvl level = e.is_critical() ? LogLvl::ERROR : LogLvl::WARN;
			LOG(level)
					<< "Line: " + to_string(k) + " in " + fileName + " Error: "
							+ e.what();

		} catch (exception &e) {
			LOG(LogLvl::ERROR)
					<< "Line: " + to_string(k) + " in " + fileName + " Error: "
							+ e.what();
		}
	}
	f.close();
}

void RuleMapper::parseString(const string &s) {
	MidiEventRule rule(s);
	rules.push_back(rule);
}

int RuleMapper::findMatchingRule(const MidiEvent &ev, int startPos) const {
	for (size_t i = startPos; i < getSize(); i++) {
		const MidiEventRule &oneRule = rules[i];
		if (oneRule.inEventRange.match(ev))
			return i;
	}
	return -1;
}

bool RuleMapper::applyRules(MidiEvent &ev) {
	// returns true if matching rule found
	bool changed = false;
	bool stop = false;

	for (size_t i = 0; i < getSize(); i++) {
		const MidiEventRule &oneRule = rules[i];
		const MidiEventRange &inEvent = oneRule.inEventRange;
		if (!inEvent.match(ev))
			continue;

		LOG(LogLvl::INFO) << "Found match for event: " << ev.toString()
				<< ", in rule: " << oneRule.toString();

		changed = true;
		switch (oneRule.rutype) {
		case MidiRuleType::STOP: {
			oneRule.outEventRange.transform(ev);
			stop = true;
			break;
		}
		case MidiRuleType::PASS: {
			oneRule.outEventRange.transform(ev);
			break;
		}
		case MidiRuleType::COUNT: {
			bool is_similar = prev_orig_ev.isSimilar(ev);
			bool off_after_on = prev_orig_ev.isNoteOn() && ev.isNoteOn();
			prev_orig_ev = ev;
			if (is_similar && !off_after_on) {
				LOG(LogLvl::DEBUG) << "Same original event ignored";
				changed = false;
			} else {
				bool is_on = ev.isNoteOn();
				MidiEvent count_ev = ev;
				oneRule.outEventRange.transform(count_ev);
				assert(count_ev.isNote() && ev.isNote());
				update_count(count_ev);
				if (is_on) {
					thread(&RuleMapper::count_and_send, this, count_ev,
							count_on).detach();
				}
			}
			stop = true;
			break;
		}
		default:
			throw MidiAppError("Unknown rule type: " + oneRule.toString());

		}

		if (stop)
			break;
	}
	return changed;
}
void RuleMapper::update_count(const MidiEvent &ev) {
	// if we got another note number, restart count
	if (!prev_count_ev.isSimilar(ev)) {
		LOG(LogLvl::DEBUG) << "New count event, count reset: "
				<< ev.toString();
		count_on = count_off = 0;
		prev_count_ev = ev;
	}

	if (ev.isNoteOn()) {
		count_on++;
	} else if (count_on > 0) {
		count_off++;
	}
}

void RuleMapper::count_and_send(const MidiEvent &ev, int cnt_on) {
	std::this_thread::sleep_for(
			std::chrono::milliseconds(RuleMapper::sleep_ms));
	if (count_on != cnt_on) {
		LOG(LogLvl::DEBUG) << "Delayed check, count changed: " << count_on
				<< " vs. " << cnt_on;
	} else if (!prev_count_ev.isSimilar(ev)) {
		LOG(LogLvl::DEBUG) << "Delayed check, new note: " << ev.toString()
				<< " vs. " << prev_count_ev.toString();
	} else {
		midi_byte_t counted_v1 = ev.v1 + count_on
				+ (count_on > count_off ? 5 : 0);
		MidiEvent e1 = ev;
		e1.v1 = counted_v1;
		count_on = count_off = 0;
		LOG(LogLvl::INFO)
				<< "Delayed check, count NOT changed, send counted note: "
				<< e1.toString();
		midi_client.send_new(e1);
	}
}

string RuleMapper::toString() const {
	ostringstream ss;
	for (size_t i = 0; i < getSize(); i++) {
		ss << "#" << i << '\t' << rules[i].toString() << endl;
	}
	return ss.str();
}