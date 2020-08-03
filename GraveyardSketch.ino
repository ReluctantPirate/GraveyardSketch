byte dungeonTier = 0;//ranges from 0 to 15
enum gameStates {SETUP, START, PLAY};
byte gameState = SETUP;
bool isStarter = false;
bool tierAssigned = false;

void setup() {

}

void loop() {

  switch (gameState) {
    case SETUP:
      setupLoop();
      break;
    case START:
      startLoop();
      break;
    case PLAY:
      playLoop();
      break;
  }

  tempDisplay();

  byte sendData = (gameState << 4) + dungeonTier;
  setValueSentOnAllFaces(sendData);
}

void setupLoop() {
  //should I be going into start?
  if (buttonMultiClicked()) {
    gameState = START;
    isStarter = true;
    tierAssigned = true;
  }

  //check neighbors for the same thing
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getGameState(getLastValueReceivedOnFace(f)) == START) {
        gameState = START;
      }
    }
  }

}

void startLoop() {
  //transition to PLAY if I'm ready for it
  if (tierAssigned) {
    gameState = PLAY;//play by default
  }

  //first, use these few frames to determine dungeon tier
  if (isStarter) {
    dungeonTier = 0;
  } else {
    byte lowestNeighborTier = 15;
    //we need to figure out which dungeon tier we are by looking at all of our neighbors
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        if (getGameState(getLastValueReceivedOnFace(f)) != SETUP) {
          //this we can evaluate
          if (getDungeonTier(getLastValueReceivedOnFace(f)) < lowestNeighborTier) {
            lowestNeighborTier = getDungeonTier(getLastValueReceivedOnFace(f));
          }
        }
      }
    }
    dungeonTier = lowestNeighborTier + 1;
    if (dungeonTier > 15) {
      dungeonTier = 15;
    }
    tierAssigned = true;
  }

  //now we check if we should move into the next phase
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getGameState(getLastValueReceivedOnFace(f)) == SETUP) {//stil doesn't know what we're doing
        gameState = START;//revert to START
      }
    }
  }

}

void playLoop() {
  //should I be going into setup?
  if (buttonMultiClicked()) {
    gameState = SETUP;
    isStarter = false;
    dungeonTier = 15;
    tierAssigned = false;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getGameState(getLastValueReceivedOnFace(f)) == SETUP) {
        gameState = SETUP;
        isStarter = false;
        dungeonTier = 15;
        tierAssigned = false;
      }
    }
  }
}

byte getDungeonTier(byte data) {
  return (data & 15);
}

byte getGameState(byte data) {
  return ((data >> 4) & 3);
}

void tempDisplay() {
  switch (gameState) {
    case SETUP:
      setColor(CYAN);
      break;
    case START:
      setColor(RED);
      break;
    case PLAY:
      setColor(dim(WHITE, 100));
      FOREACH_FACE(f) {
        if (dungeonTier > f) {
          setColorOnFace(YELLOW, f);
        }
      }
      if (dungeonTier == 15) {
        setColor(GREEN);
      }
      break;
  }
}
