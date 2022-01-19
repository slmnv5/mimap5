#ifndef MIDIFILTER_H
#define MIDIFILTER_H
#include "pch.hpp"
#include "RuleMapper.hpp"
#include "NoteCounter.hpp"

#include <chrono>

using namespace std;

class MidiFilter {
protected:
	int client = -1;
	int inport = -1;
	int outport = -1;
	snd_seq_t *seq_handle = nullptr;

public:
	MidiFilter() {
	}
	virtual ~MidiFilter() {
	}

	void send_event(snd_seq_event_t *event) const;
	void open_alsa_connection();
	void process_events(long count);
	void send_new(const MidiEvent &ev) const;
	virtual void process_one_event(snd_seq_event_t *event, MidiEvent &ev) {
	}
};
//=============== class that maps in event to out events ============================
class MidiFilterRule: public MidiFilter {
private:
	const RuleMapper rule_mapper;

public:
	MidiFilterRule(const string &ruleFile) :
			rule_mapper(ruleFile) {
	}
	virtual ~MidiFilterRule() {
	}

	virtual void process_one_event(snd_seq_event_t *event, MidiEvent &ev);
};

//========= class that convert CC to notes and count in events =======================
class MidiFilterCount: public MidiFilter {
	/*Same as parent but converts CC to notes.
	 * When  CC value increases first time, it makes note ON and ignores next increases.
	 * When CC value decrease it makes note OFF and ignores next decreases.
	 * When new channel or CC number comes all is reset.
	 * Generated note number is same as CC number */

	typedef std::chrono::steady_clock the_clock;
	typedef the_clock::time_point time_pt;
	typedef std::chrono::milliseconds millis;

	millis millis_600 { 600 };

private:
	time_pt last_moment = the_clock::now();

	MidiEvent last_ev;
	int count_on = 0;
	int count_off = 0;
	NoteCounter note_counter;
	bool not_similar_or_delayed(const MidiEvent &ev);
public:
	MidiFilterCount(const string &fileName) :
			note_counter(fileName) {
	}
	~MidiFilterCount() {
	}
	virtual void process_one_event(snd_seq_event_t *event, MidiEvent &ev);
	void send_event_delayed(MidiEvent ev, int c_on, int c_off);
};

#endif
