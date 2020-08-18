byte dungeonTier = 5;//ranges from 0 to 7
enum gameStates {SETUP, START, PLAY};
byte gameState = SETUP;
bool isStarter = false;
bool tierAssigned = false;

bool isRevealed = false;
enum revealType {PLAIN, TRAP, REWARD};
byte revealType = PLAIN;

void setup() {

}

void loop() {

  switch (gameState) {
    case SETUP:
      setupLoop();
      setupDisplay();
      break;
    case START:
      startLoop();
      startDisplay();
      break;
    case PLAY:
      playLoop();
      playDisplay();
      break;
  }

  byte sendData = (gameState << 3) + dungeonTier;
  setValueSentOnAllFaces(sendData);

  buttonSingleClicked();
  buttonDoubleClicked();
  buttonMultiClicked();
  buttonLongPressed();
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
  isRevealed = false;
  revealType = PLAIN;
  //transition to PLAY if I'm ready for it
  if (tierAssigned) {
    gameState = PLAY;//play by default
  }

  //first, use these few frames to determine dungeon tier
  if (isStarter) {
    dungeonTier = 0;
  } else {
    byte lowestNeighborTier = 7;
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
    if (dungeonTier > 5) {
      dungeonTier = 5;
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
    dungeonTier = 5;
    tierAssigned = false;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getGameState(getLastValueReceivedOnFace(f)) == SETUP) {
        gameState = SETUP;
        isStarter = false;
        dungeonTier = 5;
        tierAssigned = false;
      }
    }
  }

  if (buttonSingleClicked() && dungeonTier != 0 && revealType != TRAP) {//reveal!
    //decide what's behind the fog
    //are we something special?

    //so we have a series of probabilities, but we default to red
    revealType = TRAP;
    switch (dungeonTier) {
      case 1:
        if (random(100) < 75) {
          revealType = REWARD;
        }
        break;
      case 2:
        if (random(100) < 60) {
          revealType = REWARD;
        }
        break;
      case 3:
        if (random(100) < 50) {
          revealType = REWARD;
        }
        break;
      case 4:
        if (random(100) < 40) {
          revealType = REWARD;
        }
        break;
      case 5:
        if (random(100) < 30) {
          revealType = REWARD;
        }
        break;
    }

    //    if (random(7) < dungeonTier) {//this nearly guarantees a special thing on the far ones
    //      if (random(1) == 0) {
    //        revealType = REWARD;
    //      } else {
    //        revealType = TRAP;
    //      }
    //    } else {
    //      revealType = PLAIN;
    //    }

    isRevealed = true;
  }
}

byte getDungeonTier(byte data) {
  return (data & 7);
}

byte getGameState(byte data) {
  return ((data >> 3) & 3);
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
      if (dungeonTier == 5) {
        setColor(GREEN);
      }
      break;
  }
}

void setupDisplay() {
  byte spiralOffset = (millis() / 250) % 6;

  FOREACH_FACE(f) {
    setColorOnFace(dim(WHITE, 50 * f), (f + spiralOffset) % 6);
  }
}

void startDisplay() {
  setColor(WHITE);
}

Color rewardColors[5] = {BLUE, GREEN, ORANGE, YELLOW, WHITE};

void playDisplay() {
  if (dungeonTier == 0) {//the staircase
    FOREACH_FACE(f) {
      setColorOnFace(dim(BLUE, 50 * f), f);
    }
  } else {
    if (isRevealed) {//actual display
      switch (revealType) {
        case PLAIN:
          setColor(dim(BLUE, 50));
          setColorOnFace(dim(ORANGE, 50), 0);
          setColorOnFace(dim(ORANGE, 50), 2);
          setColorOnFace(dim(ORANGE, 50), 4);
          break;
        case TRAP:
          setColor(RED);
          setColorOnFace(ORANGE, random(5));
          break;
        case REWARD:
          setColor(rewardColors[dungeonTier - 1]);
          setColorOnFace(dim(rewardColors[dungeonTier - 1], 150), random(5));
          setColorOnFace(dim(rewardColors[dungeonTier - 1], 150), random(5));
          break;
      }
    } else {//fog of war
      setColor(dim(BLUE, 25));
      setColorOnFace(dim(ORANGE, 25), random(5));
    }
  }
}
