#include <JsonParser.h>
#include <unwind-cxx.h>
#include <system_configuration.h>
#include <utility.h>
#include <StandardCplusplus.h>
#include <vector>
#include <Wire.h>
#include <nunchuck_funcs.h>
#include <SoftwareSerial.h>
#include <MemoryFree.h>

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
Song songs[8];
vector<int> notesPlayed;
int NOTE_LIMIT = 10;

char* songJson[] = {
  "{\"name\":\"Song Of Time\",\"notes\":[69,62,65,69,62,65,69,72,71,67,65,67,69,62,60,64,62],\"times\":[600,900,600,600,900,600,300,300,600,600,300,300,600,600,300,300,900],\"nunchuck\":[3,1,2,3,1,2]}",
  "{\"name\":\"Inverted Song of Time\",\"notes\":[65,62,69,65,62,69,64,63,62,61,60,59],\"times\":[600,900,600,600,900,600,900,300,300,300,600,900],\"nunchuck\":[2,1,3,2,1,3]}",
  "{\"name\":\"Epona's Song\",\"notes\":[74,71,69,74,71,69,74,71,69,71,69],\"times\":[300,300,1200,300,300,1200,300,300,600,600,900],\"nunchuck\":[5,4,3,5,4,3]}",
  "{\"name\":\"Song of Healing\",\"notes\":[71,69,65,71,69,65,71,69,64,62,64],\"times\"[600,600,600,600,600,600,600,600,300,300,1200],\"nunchuck\":[4,3,2,4,3,2]}",
  "{\"name\":\"Song of Double Time\",\"notes\":[69,69,62,62,65,65,69,69,62,62,65,65,70,70,67,67,70,70,73,73,69,69,76,77],\"times\":[300,280,260,240,220,220,200,200,180,180,160,160,140,140,120,120,100,100,100,100,100,100,300,900],\"nunchuck\":[3,3,1,1,2,2]}",
  "{\"name\":\"Song of Soaring\",\"notes\":[65,71,74,65,71,74],\"times\":[300,300,600,300,300,900],\"nunchuck\":[2,4,5,2,4,5]}",
  "{\"name\":\"Elegy of Emptiness\",\"notes\":[69,71,69,65,69,74,71],\"times\":[600,200,400,400,400,400,800],\"nunchuck\":[3,4,3,2,3,5,4]}",
  "{\"name\":\"Oath to Order\",\"notes\":[69,65,62,65,69,74],\"times\":[800,400,400,400,400,800],\"nunchuck\":[3,2,1,2,3,5]}"
};

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
    }  
    else if (nunchuck_joyy() < 55) {
      noteOn(0, 65, 90);
      note = 2;
    } 
    else if (nunchuck_joyx() > 200) {
      noteOn(0, 69, 90);
      note = 3;
    } 
    else if (nunchuck_joyx() < 55) {
      noteOn(0, 71, 90);
      note = 4;
    } 
    else if (nunchuck_joyy() > 200) {
      noteOn(0, 74, 90);
      note = 5;
    } 
  }
  else {
    if (note == 1 && !nunchuck_cbutton()) {
      noteOff(0, 62, 90);
      note = 0;
      updateAndDetectSong(1);
    } 
    else if (note == 2 && nunchuck_joyy() >= 55) {
      noteOff(0, 65, 90);
      note = 0;
      updateAndDetectSong(2);
    } 
    else if (note == 3 && nunchuck_joyx() <= 200) {
      noteOff(0, 69, 90);
      note = 0;
      updateAndDetectSong(3);
    } 
    else if (note == 4 && nunchuck_joyx() >= 55) {
      noteOff(0, 71, 90);
      note = 0;
      updateAndDetectSong(4);
    } 
    else if (note == 5 && nunchuck_joyy() <= 200) {
      noteOff(0, 74, 90);
      note = 0;
      updateAndDetectSong(5);
    }
  }
  delay(1);
}

void generateSongs() {
  JsonParser<64> parser;
  for (int i = 0; i < sizeof(songs)/sizeof(Song); i++) {
    Serial.println(songJson[i]);
    JsonHashTable song = parser.parseHashTable(songJson[i]);
    if (song.success()) {
      songs[i].name = songs[i].name = song.getString("name");
      JsonArray noteData = song.getArray("notes");
      JsonArray timeData = song.getArray("times");
      for (int j = 0; j < noteData.getLength(); j++) {
        songs[i].notes.push_back((int)noteData.getLong(j));
        songs[i].times.push_back((int)timeData.getLong(j));
      }
      JsonArray nunchuckData = song.getArray("nunchuck");
      for (int j = 0; j < nunchuckData.getLength(); j++) {
        songs[i].nunchuck_notes.push_back((int)nunchuckData.getLong(j));
      }
      Serial.println(songs[i].name);
      for (int j = 0; j < songs[i].notes.size(); j++) {
        Serial.println(songs[i].notes.at(j));
      }
      Serial.println();
      for (int j = 0; j < songs[i].nunchuck_notes.size(); j++) {
        Serial.println(songs[i].nunchuck_notes.at(j));
      }
      Serial.print("Free Memory = ");
      Serial.println(getFreeMemory());
    } 
    else {
      Serial.println("Json error");
    }
  }

  /*songs[i].name = "Song of Time";
   int songOfTimeNotes[17] = {69, 62, 65, 69, 62, 65, 69, 72, 71, 67, 65, 67, 69, 62, 60, 64, 62};
   int songOfTimeTimes[17] = {600, 900, 600, 600, 900, 600, 300, 300, 600, 600, 300, 300, 600, 600, 300, 300, 900};
   for (int i = 0; i < sizeof(songOfTimeNotes) / sizeof(int); i++) {
   songs[i].notes.push_back(songOfTimeNotes[i]);
   songs[i].times.push_back(songOfTimeTimes[i]);
   }
   int songOfTimeNunchuck[6] = {3, 1, 2, 3, 1, 2};
   for (int i = 0; i < sizeof(songOfTimeNunchuck) / sizeof(int); i++) {
   songs[i].nunchuck_notes.push_back(songOfTimeNunchuck[i]);
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
  if (notesPlayed.size() > NOTE_LIMIT) {
    notesPlayed.erase(&notesPlayed.at(0));
    Serial.println("Note removed.");
  }
  for (int i = 0; i < notesPlayed.size(); i++) {
    Serial.println(notesPlayed.at(i));
  }
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

