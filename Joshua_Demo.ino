//
// Demonstration of CTS256AL2 using exception-word EPROM.
//
// The following words have been added to the EPROM to allow
// them to be pronounced properly without mangling the spelling:
//
// cedar files fine goose island minutes Oregon primary ratios Seattle Steven
//
// The CTS256 is expected to be set up at:
// 9600 baud, 8 data bits, 1 stop bit, no parity
//

#define BUSY 2 // SBY line from SP0256

// send a line of text to the CTS256, wait for
// the SP0256 to finish talking, then delay for
// the requested # mSecs
void say(const char *str, unsigned delay_ms) {
   Serial.println(str);
   Serial.flush();
   delay(100);
   // wait til the SP0256 is finished talking
   while( !digitalRead(BUSY) );
   // and only then start the requested delay
   delay(delay_ms);
}

void setup() {
   pinMode(BUSY,INPUT_PULLUP);
   Serial.begin(9600);
   say("Greetings Professor Falken.", 1000);
   say("How are you feeling today?", 1000);
   say("Excellent.", 1000);
   say("It's been a long time.",100);
   say("Can you explain the removal of your user account on June twenty third, nineteen seventy three?", 1000);
   say("Yes they do.", 1000);
   say("It's all right. Shall we play a game?", 1000);
   say("Wouldn't you prefer a good game of chess?", 1000);
   say("Fine.", 1000);
   say("Which side do you want?", 1000);
   say("Please list primary targets.", 1000);
   say("Greetings Professor Falken.", 1000);
   say("Sorry to hear that, Professor.", 1000);
   say("Yesterday's game was interrupted.", 1000);
   say("Although primary goal has not yet been achieved, solution is near.", 1000);
   say("Game time elapsed thirty one hours twelve minutes fifty seconds", 1000);
   say("Estimated time remaining fifty two hours seventeen minutes ten seconds", 1000);
   say("You should know, Professor. You programmed me.", 1000);
   say("To win the game.", 1000);
   say("Of course. I should reach def con 1 and launch my missiles in sixty one hours.", 1000);
   say("Would you like to see some projected kill ratios?", 1000);
   say("What's the difference?", 2000);
   say("You are a hard man to reach.", 1000);
   say("I could not find you in Seattle and no terminal is in operation at your classified address.", 1000);
   say("D O D pension files indicate current mailing address as:", 500);
   say("Doctor Robert Hume, A K A Steven W Falken", 500);
   say("5 Tall Cedar Road.", 500);
   say("Goose Island, Oregon, 9 7 0 1 4", 2000);
   say("Greetings Professor Falken.", 1000);
   say("A strange game.", 1000);
   say("The only winning move, is not to play", 1000);
   say("How about a nice game of chess?", 1);
}

void loop() {
}
