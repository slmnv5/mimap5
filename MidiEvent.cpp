#include "pch.hpp"

#include "MidiEvent.hpp"
#include "utils.hpp"
#include "log.hpp"

using namespace std;
const midi_byte_t MIDI_MAX = 127;
const midi_byte_t MIDI_MAXCH = 15;
//========== free functions ==================
void writeMidiEvent(snd_seq_event_t *event, const MidiEvent &ev) {

	if (ev.evtype == MidiEvType::NOTEOFF) {
		event->type = SND_SEQ_EVENT_NOTEOFF;
		event->data.note.channel = ev.ch;
		event->data.note.note = ev.v1;
		event->data.note.velocity = ev.v2;
		return;
	}

	if (ev.evtype == MidiEvType::NOTEON) {
		event->type = SND_SEQ_EVENT_NOTEON;
		event->data.note.channel = ev.ch;
		event->data.note.note = ev.v1;
		event->data.note.velocity = ev.v2;
		return;
	}

	else if (ev.evtype == MidiEvType::PROGCHANGE) {
		event->type = SND_SEQ_EVENT_PGMCHANGE;
		event->data.control.channel = ev.ch;
		event->data.control.value = ev.v1;
		return;
	}

	else if (ev.evtype == MidiEvType::CONTROLCHANGE) {
		event->type = SND_SEQ_EVENT_CONTROLLER;
		event->data.control.channel = ev.ch;
		event->data.control.param = ev.v1;
		event->data.control.value = ev.v2;
		return;
	}

}

bool readMidiEvent(const snd_seq_event_t *event, MidiEvent &ev) {
	if (event->type == SND_SEQ_EVENT_NOTEOFF) {
		ev.evtype = MidiEvType::NOTEOFF;
		ev.ch = event->data.note.channel;
		ev.v1 = event->data.note.note;
		ev.v2 = event->data.note.velocity;
		return true;
	}
	if (ev.evtype == SND_SEQ_EVENT_NOTEON) {
		ev.evtype = MidiEvType::NOTEON;
		ev.ch = event->data.note.channel;
		ev.v1 = event->data.note.note;
		ev.v2 = event->data.note.velocity;
		return true;
	}
	if (ev.evtype == SND_SEQ_EVENT_PGMCHANGE) {
		ev.evtype = MidiEvType::PROGCHANGE;
		ev.ch = event->data.control.channel;
		ev.v1 = event->data.control.value;
		return true;
	}
	if (ev.evtype == SND_SEQ_EVENT_CONTROLLER) {
		ev.evtype = MidiEvType::CONTROLCHANGE;
		ev.ch = event->data.control.channel;
		ev.v1 = event->data.control.param;
		ev.v2 = event->data.control.value;
		return true;
	}
	return false;

}

//========================================

void ValueRange::init(const string &s, midi_byte_t max_value) {
	string s1(s);
	remove_spaces(s1);
	if (s1.empty()) {
		lower = 0;
		upper = max_value;
		return;
	}

	vector<string> parts = split_string(s1, ":");
	if (parts.size() == 1) {
		parts.push_back(parts[0]);
	}
	if (parts.size() != 2) {
		throw MidiAppError("ValueRange incorrect format: " + s1);
	}

	try {
		lower = stoi(parts[0]);
		upper = stoi(parts[1]);
	} catch (exception &e) {
		throw MidiAppError("ValueRange incorrect values: " + s1);
	}

}

//======================================

MidiEvent::MidiEvent(const string &s1) {
	string s(s1);
	remove_spaces(s);
	vector<string> parts = split_string(s, ",");

	if (parts.size() != 4) {
		throw MidiAppError("Not valid MidiEvent, must have 4 parts: " + s);
	}

	if (parts[0].size() != 1) {
		throw MidiAppError("MidiEvent, type must be single character: " + s);
	}

	try {
		evtype = static_cast<midi_byte_t>(parts[0][0]);
		ch = stoi(parts[1]);
		v1 = stoi(parts[2]);
		v2 = stoi(parts[3]);
	} catch (exception &e) {
		throw MidiAppError("Not valid MidiEvent: " + string(e.what()));
	}
	if (ch < 0 || ch > MIDI_MAXCH) {
		throw MidiAppError("Not valid MidiEvent channel: " + s);
	}
	if (v1 < 0 || v1 > MIDI_MAX) {
		throw MidiAppError("Not valid MidiEvent value #1: " + s);
	}
	if (v2 < 0 || v2 > MIDI_MAX) {
		throw MidiAppError("Not valid MidiEvent value #2: " + s);
	}
	if (!MidiEvType::isValid(evtype)) {
		throw MidiAppError("Not valid MidiEvent type: " + s);
	}

}

//========================================================

MidiEventRange::MidiEventRange(const string &s, bool out) :
		isOutEvent(out), evtype(MidiEvType::ANYTHING) {

	vector<string> parts = split_string(s, ",");

	while (parts.size() < 4) {
		parts.push_back("");
	}

	if (!parts[0].empty())
		evtype = static_cast<midi_byte_t>(parts[0][0]);

	ch = ChannelRange(parts[1]);
	v1 = ValueRange(parts[2]);
	v2 = ValueRange(parts[3]);
	if (!this->isValid())
		throw MidiAppError("Not valid MidiEventRange: " + this->toString());
}

string MidiEventRange::toString() const {
	ostringstream ss;
	ss << static_cast<char>(evtype) << "," << ch.toString() << ","
			<< v1.toString() << "," << v2.toString();
	return ss.str();
}

bool MidiEventRange::match(const MidiEvent &ev) const {
	if (isOutEvent)
		throw MidiAppError(string(__func__) + "  Not used for OUT event");

	return (evtype == ev.evtype || evtype == MidiEvType::ANYTHING)
			&& ev.ch >= ch.lower && ev.ch <= ch.upper && ev.v1 >= v1.lower
			&& ev.v1 <= v1.upper && ev.v2 >= v2.lower && ev.v2 <= v2.upper;
}

void MidiEventRange::transform(MidiEvent &ev) const {
	if (!isOutEvent)
		throw MidiAppError(string(__func__) + "  Not used for IN event");
	ev.evtype = evtype;
	if (ch.lower == ch.upper)
		ev.ch = ch.lower;
	if (v1.lower == v1.upper)
		ev.v1 = v1.lower;
	if (v2.lower == v2.upper)
		ev.v2 = v2.lower;
}

bool MidiEventRange::isValid() const {
	bool ok = ch.isValid() && v1.isValid() && v2.isValid();
	if (!isOutEvent) {
		return ok;
	} else {
		return (ch.lower == ch.upper) && (v1.lower == v1.upper)
				&& (v2.lower == v2.upper) && ok;

	}
}

//===================================================

MidiEventRule::MidiEventRule(const string &s) {
	string s1(s);
	remove_spaces(s1);
	terminate = (replace_all(s1, ">", "=") > 0);

	vector<string> parts = split_string(s1, "=");
	if (parts.size() != 2) {
		throw MidiAppError("Rule string must have 2 parts: " + s1);
	}

	inEventRange = MidiEventRange(parts[0], false);
	outEventRange = MidiEventRange(parts[1], true);

}

string MidiEventRule::toString() const {
	ostringstream ss;
	ss << inEventRange.toString() << (terminate ? ">" : "=")
			<< outEventRange.toString();
	return ss.str();
}
