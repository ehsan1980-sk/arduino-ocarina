#include <unwind-cxx.h>
#include <system_configuration.h>
#include <utility.h>
#include <StandardCplusplus.h>
#include <vector>
#include <Wire.h>
#include <nunchuck_funcs.h>
#include <SoftwareSerial.h>
#include <aJSON.h>

using namespace std;

class Song {
  public:
    String name;
    vector<int> notes;
    vector<int> nunchuck_notes;
    vector<int> times;
    void playSong();
};

void Song::playSong() {
  for (int i = 0; i < notes.size(); i++) {
    noteOn(0, notes.at(i), 90);
    delay(times[i]);
    noteOff(0, notes.at(i), 90);
  }
}

SoftwareSerial mySerial(2, 3);

byte resetMIDI = 4;
byte ledPin = 13;
int instrument = 0;
int note = 0;
Song songs[2];
vector<int> notesPlayed;
int NOTE_LIMIT = 10;

char* songText = "{"
	"\"songs\": ["
		"{"
			"\"name\": \"Song Of Time\","
			"\"notes\": [69, 62, 65, 69, 62, 65, 69, 72, 71, 67, 65,"
"67, 69, 62, 60, 64, 62],"
			"\"times\": [600, 900, 600, 600, 900, 600, 300, 300, 600,"
"600, 300, 300, 600, 600, 300, 300, 900],"
			"\"nunchuck\": [3, 1, 2, 3, 1, 2]"	
		"}"
	"]"
"}";

void setup()
{
  mySerial.begin(31250);
  Serial.begin(19200);
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0x07, 80);
  
  nunchuck_setpowerpins();
  nunchuck_init();
  
  talkMIDI(0xB0, 0, 0x00);
  talkMIDI(0xC0, 77, 0);
  
  generateSongs();
}

void loop() {
  nunchuck_get_data();
  if (note == 0) {
    if (nunchuck_cbutton()) {
      noteOn(0, 62, 90);
      note = 1;
    }  else if (nunchuck_joyy() < 55) {
      noteOn(0, 65, 90);
      note = 2;
    } else if (nunchuck_joyx() > 200) {
      noteOn(0, 69, 90);
      note = 3;
    } else if (nunchuck_joyx() < 55) {
      noteOn(0, 71, 90);
      note = 4;
    } else if (nunchuck_joyy() > 200) {
      noteOn(0, 74, 90);
      note = 5;
    } 
  }
  else {
    if (note == 1 && !nunchuck_cbutton()) {
      noteOff(0, 62, 90);
      note = 0;
      updateAndDetectSong(1);
    } else if (note == 2 && nunchuck_joyy() >= 55) {
      noteOff(0, 65, 90);
      note = 0;
      updateAndDetectSong(2);
    } else if (note == 3 && nunchuck_joyx() <= 200) {
      noteOff(0, 69, 90);
      note = 0;
      updateAndDetectSong(3);
    } else if (note == 4 && nunchuck_joyx() >= 55) {
      noteOff(0, 71, 90);
      note = 0;
      updateAndDetectSong(4);
    } else if (note == 5 && nunchuck_joyy() <= 200) {
      noteOff(0, 74, 90);
      note = 0;
      updateAndDetectSong(5);
    }
  }
  delay(1);
}

void generateSongs() {
  aJsonObject* root = aJson.parse(songText);
  Serial.println(aJson.print(root));
  aJsonObject* songData = aJson.getObjectItem(root, "songs");
  aJsonObject* song = aJson.getArrayItem(songData, 0);
  aJsonObject* songName = aJson.getObjectItem(song, "name");
  songs[0].name = songName->valuestring;
  aJsonObject* noteData = aJson.getObjectItem(song, "notes");
  aJsonObject* timeData = aJson.getObjectItem(song, "times"); 
  int numberNotes = aJson.getArraySize(noteData);
  for (int i = 0; i < numberNotes; i++) {
    aJsonObject* note = aJson.getArrayItem(noteData, i);
    aJsonObject* time = aJson.getArrayItem(timeData, i);
    songs[0].notes.push_back(note->valueint);
    songs[0].notes.push_back(time->valueint);
  }
  aJsonObject* nunchuckData = aJson.getObjectItem(song, "nunchuck");
  int numberNunchuck = aJson.getArraySize(nunchuckData);
  for (int i = 0; i < numberNunchuck; i++) {
    aJsonObject* note = aJson.getArrayItem(nunchuckData, i);
    songs[0].nunchuck_notes.push_back(note->valueint);
  }
  Serial.println(songs[0].name);
  for (int i = 0; i < songs[0].notes.size(); i++) {
    Serial.println(songs[0].notes.at(i));
  }
  Serial.println();
  for (int i = 0; i < songs[0].nunchuck_notes.size(); i++) {
    Serial.println(songs[0].nunchuck_notes.at(i));
  }
  
  /*songs[0].name = "Song of Time";
  int songOfTimeNotes[17] = {69, 62, 65, 69, 62, 65, 69, 72, 71, 67, 65, 67, 69, 62, 60, 64, 62};
  int songOfTimeTimes[17] = {600, 900, 600, 600, 900, 600, 300, 300, 600, 600, 300, 300, 600, 600, 300, 300, 900};
  for (int i = 0; i < sizeof(songOfTimeNotes) / sizeof(int); i++) {
    songs[0].notes.push_back(songOfTimeNotes[i]);
    songs[0].times.push_back(songOfTimeTimes[i]);
  }
  int songOfTimeNunchuck[6] = {3, 1, 2, 3, 1, 2};
  for (int i = 0; i < sizeof(songOfTimeNunchuck) / sizeof(int); i++) {
    songs[0].nunchuck_notes.push_back(songOfTimeNunchuck[i]);
  }
  
  songs[1].name = "Inverted Song of Time";
  int invertedSongOfTimeNotes[12] = {65, 62, 69, 65, 62, 69, 64, 63, 62, 61, 60, 59};
  int invertedSongOfTimeTimes[12] = {600, 900, 600, 600, 900, 600, 900, 300, 300, 300, 600, 900};
  for (int i = 0; i < sizeof(invertedSongOfTimeNotes) / sizeof(int); i++) {
    songs[1].notes.push_back(invertedSongOfTimeNotes[i]);
    songs[1].times.push_back(invertedSongOfTimeTimes[i]);
  }
  int invertedSongOfTimeNunchuck[6] = {2, 1, 3, 2, 1, 3};
  for (int i = 0; i < sizeof(invertedSongOfTimeNunchuck) / sizeof(int); i++) {
    songs[1].nunchuck_notes.push_back(invertedSongOfTimeNunchuck[i]);
  }*/
}

void updateAndDetectSong(int notePlayed) {
  notesPlayed.push_back(notePlayed);
  if (notesPlayed.size() > NOTE_LIMIT)
    notesPlayed.erase(&notesPlayed.at(0));
  /*for (int i = 0; i < notesPlayed.size(); i++) {
    Serial.print(notesPlayed.at(i));
  }*/
  for (int s = 0; s < sizeof(songs) / sizeof(Song); s++) {
    if (notesPlayed.size() >= songs[s].nunchuck_notes.size()) {
      boolean songsEqual = true;
      for (int i = 0; i < songs[s].nunchuck_notes.size(); i++) {
        if (songs[s].nunchuck_notes.at(songs[s].nunchuck_notes.size() - 1 - i) != notesPlayed.at(notesPlayed.size() - 1 - i)) {
          songsEqual = false;
          break;
        }
      }
      if (songsEqual) {
        songs[s].playSong();
        notesPlayed.clear();
      }
    }
  }
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}
