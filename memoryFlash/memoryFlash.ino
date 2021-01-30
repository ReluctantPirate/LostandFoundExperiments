/*
 * HOW TO PLAY
 * 
 * Blinks boot in setup mode (4 colors)
 * Double click one Blink to go into Game mode
 * Each Blink will slowly pulse it's "lost" color
 * Then it will begin to peridoically flash random colors
 * Click the Blink when the random color matches the "lost" color
 * A correct click will freeze the color and add little white correct marks
 * The game ends when you get every Blink to that state
 * Triple-click any Blink to send them all back to setup mode
 * Good luck!
 * 
 */

enum gameStates {SETUP, PLAY, END};
byte gameState = SETUP;

Timer presentationTimer;
#define PRESENTATION_INTERVAL 2000

Timer shuffleTimer;
#define SHUFFLE_INTERVAL 1000
#define SHUFFLE_VARIANCE 500

Color possibleColors[4] = {RED, YELLOW, GREEN, CYAN};
byte secretColor = 0;
byte shuffleColor = 0;

bool correctAnswer = false;

void setup() {
  randomize();
}

void loop() {
  switch (gameState) {
    case SETUP:
      setupLoop();
      setupDisplay();
      break;
    case PLAY:
      playLoop();
      playDisplay();
      break;
    case END:
      endLoop();
      endDisplay();
      break;
  }

  setValueSentOnAllFaces(gameState);
}

void setupLoop() {
  //listen to be told to start via clicking
  if (buttonDoubleClicked()) {
    gameState = PLAY;
    presentationTimer.set(PRESENTATION_INTERVAL);
    secretColor = random(3);
    correctAnswer = false;
  }

  //listen for neighbors telling me to start
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getLastValueReceivedOnFace(f) == PLAY) {
        gameState = PLAY;
        presentationTimer.set(PRESENTATION_INTERVAL);
        secretColor = random(3);
        correctAnswer = false;
      }
    }
  }
}

void playLoop() {

  //listen for multi-clicks to go back to SETUP
  if (buttonMultiClicked() && buttonClickCount() == 3) {
    gameState = END;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getLastValueReceivedOnFace(f) == END) {
        gameState = END;
      }
    }
  }

  if (!presentationTimer.isExpired()) {//show intended color
    //we don't actually do anything logically here
  } else {//do the game playing part
    if (shuffleTimer.isExpired()) {
      shuffleColor = random(3);
      shuffleTimer.set(SHUFFLE_INTERVAL + random(SHUFFLE_VARIANCE));
    } else {
      if (!correctAnswer) {//if my answer is not yet found, keep looking
        //listen for clicking
        if (buttonSingleClicked()) {
          if (secretColor == shuffleColor) {//hey, we made the right choice!
            correctAnswer = true;
          }
        }
      }
    }
  }
}

void endLoop() {

  gameState = SETUP;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getLastValueReceivedOnFace(f) == PLAY) {//oh, this one's not ready to move on
        gameState = END;//stay in END
      }
    }
  }
}

void setupDisplay() {
  setColorOnFace(possibleColors[0], 0);
  setColorOnFace(possibleColors[1], 1);
  setColorOnFace(possibleColors[2], 3);
  setColorOnFace(possibleColors[3], 4);
}

void playDisplay() {

  byte currentBrightness = 0;

  if (!presentationTimer.isExpired()) {//show the secret color once

    if (presentationTimer.getRemaining() > PRESENTATION_INTERVAL / 2) {
      currentBrightness = map(PRESENTATION_INTERVAL - presentationTimer.getRemaining(), 0, PRESENTATION_INTERVAL / 2, 0, 255);
      //currentBrightness = 255;
    } else {
      currentBrightness = map(presentationTimer.getRemaining(), 0, PRESENTATION_INTERVAL / 2, 0, 255);
    }

    setColor(dim(possibleColors[secretColor], currentBrightness));

  } else {//just show the shuffle color with a little fade on either end

    if (shuffleTimer.getRemaining() < SHUFFLE_INTERVAL / 2) {//do the fade down
      currentBrightness = map(shuffleTimer.getRemaining(), 0, SHUFFLE_INTERVAL / 2, 0, 255);
      setColor(dim(possibleColors[shuffleColor], currentBrightness));
    } else {//just show the color
      setColor(possibleColors[shuffleColor]);
    }

    //if the answer is already correct, stop showing that stuff
    if (correctAnswer) {//just some generic victory thing
      setColor(possibleColors[secretColor]);
      setColorOnFace(WHITE, 0);
      setColorOnFace(WHITE, 3);
    }

  }
}

void endDisplay() {
  setColor(OFF);
}
