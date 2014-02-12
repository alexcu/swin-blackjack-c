
/// Blackjack
/// Written By Alex Cummaudo
/// 13/12/13

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/// Define program constants
static const int SUIT_SIZE = 13;
static const int MAX_DECK_SIZE = SUIT_SIZE * 4;
static const int MAX_HAND  = 10;
static const int MAX_PLAYERS = 2;

/// Defines the four different suits
typedef enum suit_type
{
    CLUBS    = 3,
    HEARTS   = 2,
    DIAMONDS = 1,
    SPADES   = 4
}
suit_type;

/// Defines the standard ranks
typedef enum rank_type
{
    ACE_LOW,
    ACE_HIGH,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING
}
rank_type;

/// Defines the constituents of a card (its suit and rank)
typedef struct card_type
{
    suit_type suit;
    rank_type rank;
}
card_type;

/// Defines a standard deck of 52 cards (13 * 4)
typedef struct deck_type
{
    card_type   *cards[MAX_DECK_SIZE]; /* deck size variable; use malloc etc. */
    int         size;
}
deck_type;

/// Defines player types:
typedef struct player_type
{
    deck_type           *hand;
    
    enum { CPU, HUMAN } locality;
    bool                staying;
}
player_type;

/// Defines simple game data
typedef struct game_data
{
    deck_type   *deck;
    
    player_type players[MAX_PLAYERS];
    player_type *winner;
    int         current_player;
    
    bool        quit;
}
game_data;

/// Logs a simple alert in the standard format
void alert(char *alertText)
{
    printf("   > %s\n", alertText);
}

/// Reads a prompt and returns the first char of the letter inputted
char prompt(char *promptText)
{
    printf("  >> %s: ", promptText);
    
    char buffer[1];
    scanf(" %1[^\n]", buffer);
    return buffer[0];
}

/// Converts an suit to a char equivalent
char *suit_to_char(suit_type suit)
{
    switch (suit)
    {
        case CLUBS:     return "\u2663";
        case HEARTS:    return "\u2665";
        case DIAMONDS:  return "\u2666";
        case SPADES:    return "\u2660";
    }
}

/// Converts a rank to a char equivalent
char *rank_to_char(rank_type rank)
{
    switch (rank)
    {
        case ACE_LOW:   return " A";
        case ACE_HIGH:  return " A";
        case TWO:       return " 2";
        case THREE:     return " 3";
        case FOUR:      return " 4";
        case FIVE:      return " 5";
        case SIX:       return " 6";
        case SEVEN:     return " 7";
        case EIGHT:     return " 8";
        case NINE:      return " 9";
        case TEN:       return "10";
        case JACK:      return " J";
        case QUEEN:     return " Q";
        case KING:      return " K";
    }
}

/// Converts a  rank to an int equivalent
int rank_to_int(rank_type rank)
{
    switch (rank)
    {
        case ACE_LOW:   return 1;
        case ACE_HIGH:  return 11;
        case TWO:       return 2;
        case THREE:     return 3;
        case FOUR:      return 4;
        case FIVE:      return 5;
        case SIX:       return 6;
        case SEVEN:     return 7;
        case EIGHT:     return 8;
        case NINE:      return 9;
        case TEN:       return 10;
        case JACK:      return 10;
        case QUEEN:     return 10;
        case KING:      return 10;
    }
}

/// Returns a character printout of the card provided
char *card_to_char(card_type card)
{
    // A pointer to char
    char *result[255];

    char *theRank = rank_to_char(card.rank);
    char *theSuit = suit_to_char(card.suit);
    sprintf(result, "%s%s", theRank, theSuit);

    // Return the pointer to 255 chars
    return result;
}

/// Calculates the value of the hand
int hand_value(deck_type *theHand)
{
    int sum = 0;
    int i;
    for (i = 0; i < theHand->size; i++)
    {
        // Switch an ace to most optimal for aces such that less than 21
        if (theHand->cards[i]->rank == ACE_LOW || theHand->cards[i]->rank == ACE_HIGH)
        {
            // Case 1: Using a low ace
            if (sum + 1 <= 21) { theHand->cards[i]->rank = ACE_LOW; }
                
            // Case 2: Using a high ace
            if (sum + 11 <= 21) { theHand->cards[i]->rank = ACE_HIGH; }
        }
        
        sum += rank_to_int(theHand->cards[i]->rank);
    }
    return sum;
}

/// Prints a deck's contents
void print_deck(deck_type *theDeck)
{
    int i;
    for (i = 0; i < theDeck->size; i++)
        printf("   > [Card #%d] => %s\n", i+1, card_to_char(*theDeck->cards[i]));
    printf("   > Total Value = %d\n", hand_value(theDeck));
}

/// Determines a random number within 0 and max lim
int rnd(int max)
{
    return rand() % max;
}

/// Shuffles the deck passed
void shuffle_deck(deck_type *theDeck)
{
    // For every card in the deck
    int i;
    for (i = 0; i < theDeck->size; i++)
    {
        // Random number between 0 and DECK_SIZE
        int swapPos = rnd(theDeck->size);
        
        // Replace the i'th card with the swapPos card
        card_type *tmpCard      = theDeck->cards[i];
        theDeck->cards[i]       = theDeck->cards[swapPos];
        theDeck->cards[swapPos] = tmpCard;
    }
}

/// Empties the deck by setting all card pointers to NULL
void empty_deck(deck_type *theDeck)
{
    int i;
    // Given the deck wasn't initialised in the first place
    if (theDeck != NULL)
    for (i = 0; i < MAX_DECK_SIZE; i++)
        theDeck->cards[i] = NULL;
}

/// Returns a new deck that populates each card
/// and then shuffles the deck
void setup_deck(deck_type *theDeck)
{
    // Cleanup deck
    empty_deck(theDeck);
    
    // Set initial deck size
    theDeck->size = MAX_DECK_SIZE;
    
    // Setup temporary suits and decks to work with
    suit_type theSuit   = 0;
    rank_type theRank   = 0;
    
    // Setup iterators
    int rankIterator    = 0;
    int suitIterator    = 0;
    int i               = 0;    /* overall card iterator */
    
    
    // Start assigning suits
    for (suitIterator = 0; suitIterator < 4; suitIterator++)
    {
        // Determine which suit to assign based on suitIterator
        switch (suitIterator)
        {
            case 0: theSuit = CLUBS;    break;
            case 1: theSuit = HEARTS;   break;
            case 2: theSuit = DIAMONDS; break;
            case 3: theSuit = SPADES;   break;
        }
        
        // Now populate the family of that suit
        for (rankIterator = 0; rankIterator < SUIT_SIZE; ++rankIterator)
        {
            // Determine which rank to assign based on suitIterator
            switch (rankIterator)
            {
                case 0: theRank  = ACE_LOW; break;
                case 1: theRank  = TWO;     break;
                case 2: theRank  = THREE;   break;
                case 3: theRank  = FOUR;    break;
                case 4: theRank  = FIVE;    break;
                case 5: theRank  = SIX;     break;
                case 6: theRank  = SEVEN;   break;
                case 7: theRank  = EIGHT;   break;
                case 8: theRank  = NINE;    break;
                case 9: theRank  = TEN;     break;
                case 10: theRank = JACK;    break;
                case 11: theRank = QUEEN;   break;
                case 12: theRank = KING;    break;
            }
            
            // Allocate space for this card
            theDeck->cards[i] = (card_type*)malloc(sizeof(card_type));
            
            // Set the suit of the i'th card
            theDeck->cards[i]->suit = theSuit;
            
            // Set the rank of the i'th card
            theDeck->cards[i]->rank = theRank;
            
            // Move onto the next card
            i++;
            
        }
    }
    //  Shuffle the deck
    shuffle_deck(theDeck);
}

/// Sets up a player with the given locality (CPU or HUMAN)
void setup_player(player_type *thePlayer, enum {CPU, HUMAN} locality)
{
    // Set players locality
    thePlayer->locality = locality;
    
    // Empty player's deck (their previous hand)
    empty_deck(thePlayer->hand);
    
    // Set their staying position as false
    thePlayer->staying = false;
}

/// Draws the topmost card from the deck
card_type *draw_card(deck_type *theDeck)
{
    card_type *topmostCard = theDeck->cards[0];
    
    // Given the topmost card is not NULL (i.e. not out of cards?)
    // Then shrink the deck and move all the cards down
    if (topmostCard != NULL)
    {
        // Deck shrinks by one size
        theDeck->size--;
        
        // Shuffle all cards down one (including last last since start at 1)
        int i;
        for (i = 1; i <= theDeck->size; i++)
            theDeck->cards[i-1] = theDeck->cards[i];
        
        // The last card (zero-based) is now NULL (empty slot!)
        theDeck->cards[theDeck->size] = NULL;
        
        // Now return the topmost card
        return topmostCard;
    }
    // Otherwise, return NULL to signify we have no more cards
    else return NULL;
}

/// Sets up first hand by drawing two cards from the game deck
void deal_first_hand(game_data *theGame)
{
    // For every player
    int i;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        player_type *thePlayer = &theGame->players[i];
        
        // Allocate space for this player's new hand
        thePlayer->hand = (deck_type*)malloc(sizeof(deck_type));
        
        // Set initial size of the player's first hand
        thePlayer->hand->size = 2;
        
        // Give the player their 2 first cards
        thePlayer->hand->cards[0] = draw_card(theGame->deck);
        thePlayer->hand->cards[1] = draw_card(theGame->deck);
    }
    
}

/// Setup the game
void setup_game(game_data *theGame)
{
    theGame->quit = false;
    theGame->winner = NULL;
    
    // Setup players (0 is cpu, 1 is human (for now))
    setup_player(&theGame->players[0], CPU);
    setup_player(&theGame->players[1], HUMAN);
    
    // Allocate space the deck and setup the deck
    theGame->deck = (deck_type*)malloc(sizeof(deck_type));
    setup_deck(theGame->deck);
    
    // Deal the first hand
    deal_first_hand(theGame);
}

/// Determines whether or not the game has a winner
bool is_winner(game_data *theGame)
{
    return theGame->winner != NULL;
}

/// Deals the next card to this player
void deal_next_card(player_type *thePlayer, game_data *theGame)
{
    // Next card (dont need +1 since zero-based) is a draw card
    thePlayer->hand->cards[thePlayer->hand->size] = draw_card(theGame->deck);
    // Increment hand size
    thePlayer->hand->size++;
}

/// Shows all the cards the given player currently has
void show_hand(player_type *thePlayer)
{
    print_deck(thePlayer->hand);
}

/// Moves onto the next player
void move_to_next_player(game_data *theGame)
{
    theGame->current_player++;
    if (theGame->current_player >= MAX_PLAYERS)
        theGame->current_player = 0;
}

/// Draws and recieves input for the menu
void menu(game_data *theGame)
{
    player_type *p = &theGame->players[theGame->current_player];
    
    // Draw the actual menu
    alert("What do you want to do?");
    alert("[D]raw");
    alert("[S]tay");
    alert("[L]ook at hand");
    alert("[Q]uit");
    
    char input = prompt("[?]");
    switch (input)
    {
        case 'q':
        case 'Q':
        {
            theGame->quit = true;
            break;
        }
        case 'd':
        case 'D':
        {
            deal_next_card(p, theGame);
            alert("You drew a card:");
            // The last card they drew
            alert(card_to_char(*p->hand->cards[p->hand->size-1]));
            break;
        }
        case 'l':
        case 'L':
        {
            show_hand(p);
            break;
        }
        case 's':
        case 'S':
        {
            p->staying = true;
            break;
        }
        case ']':
        {
            print_deck(theGame->deck);
            break;
        }
    }
}

/// Checks if all players have stayed
bool determine_staying(game_data *theGame)
{
    // If all players are staying check for a winner
    int i;
    for (i = 0; i < MAX_PLAYERS; i++)
        if (theGame->players[i].staying != true) return false;
    
    // Out of loop--return true
    return true;
}

/// Checks for bust for the player's hand
bool check_bust(player_type *thePlayer)
{
    
    // Determine bust
    int handVal = hand_value(thePlayer->hand);
    
    // Check if the player has gone bust
    if (handVal > 21)
    {
        // Appropriate message depending on the bust
        if (thePlayer->locality == CPU)
            alert("Dealer is bust!");
        else
            alert("You're bust--dealer wins!");

        return true;
    }
    
    return false;
}

/// Determine winner's player index by most valuable suit
int winner_by_suit_value(game_data *theGame)
{
    suit_type highestSuit = -1;
    int winningPlayerIndex = -1;
    
    // For every player
    int i;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        player_type *p = &theGame->players[i];
        card_type *highestCard = p->hand->cards[0];
        
        // Get this player's highest rank in their hand
        int j;
        for (j = 1; j < p->hand->size; j++)
        {
            if (p->hand->cards[j]->rank >= highestCard->rank)
                highestCard = p->hand->cards[j];
        }
        
        // If the highest card exceeds the highest suit
        if (highestCard->suit > highestSuit)
        {
            highestSuit = highestCard->suit;
            winningPlayerIndex = i;
        }
    }
    
    return winningPlayerIndex;
}

/// Determine winner's player index by least cards drawn
int winner_by_small_hand(game_data *theGame)
{
    // Determine the hand sizes for all players
    int smallestHandSize = -1;
    int winningPlayerIndex = -1;
    int i;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        int ithHandSize = theGame->players[i].hand->size;
        // Check who has the greater hand size
        if (ithHandSize < smallestHandSize)
        {
            smallestHandSize = ithHandSize;
            winningPlayerIndex = i;
        }
        // Checking for draw
        else if (ithHandSize == smallestHandSize)
        {
            // Returns -1 for indeterminate
            return -1;
        }
    }
    return winningPlayerIndex;
}

/// Determine winner's player index by value
int winner_by_hand_value(game_data *theGame)
{
    // Determine the hand values for all players
    int largestHandValue = -1;
    int winningPlayerIndex = -1;
    int i;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        int ithHandValue = hand_value(theGame->players[i].hand);
        // Check who has the greater hand
        if (ithHandValue > largestHandValue && ithHandValue <= 21)
        {
            largestHandValue = ithHandValue;
            winningPlayerIndex = i;
        }
        // Checking for draw
        else if (ithHandValue == largestHandValue)
        {
            // Returns -1 for indeterminate
            return -1;
        }
    }
    return winningPlayerIndex;
}

/// Determines the winner and sets their win status to true
player_type *determine_winner(game_data *theGame)
{
    int winningPlayerIndex = winner_by_hand_value(theGame);
    
    // Determination by hand value failed?
    if (winningPlayerIndex == -1)
    {
        // Try hand size
        winningPlayerIndex = winner_by_small_hand(theGame);
        
        // Determination by hand size failed?
        if (winningPlayerIndex == -1)
        {
            winningPlayerIndex = winner_by_suit_value(theGame);
        }
    }
    
    return &theGame->players[winningPlayerIndex];
}

/// Performs a round
void perform_round(game_data *theGame)
{
    player_type *p = &theGame->players[theGame->current_player];
    
    // Human's turn
    if (!p->locality == CPU)
    {
        // Print menu given we aren't staying
        menu(theGame);
    }
    // CPU's turn
    else
    {
//        // 25% chance that CPU won't cheat and look at the
//        // next card in the deck
//        if (rnd(100) < 75)
        {
            // While CPU's hand plus the next card's rank <= 21
            while ((hand_value(p->hand) + rank_to_int(theGame->deck->cards[0]->rank) <= 21))
                deal_next_card(p, theGame);
        }
//        // Otherwise just deal rnd(3) times
//        else
//        {
//            int i;
//            for (i = rnd(3); i > 0; i--)
//                deal_next_card(p, theGame);
//        }
        p->staying = true;
    }
    
    bool busted = check_bust(p);
    
    // Break if *all* players are staying or
    // if we have a bust on the current player
    if (determine_staying(theGame) || busted)
    {
        // Determine the winner
        theGame->winner = determine_winner(theGame);
        // Break
        return;
    }
    
    // Move onto the next player if a bust has occured or
    // if the current player is staying
    if (p->staying || busted)
        move_to_next_player(theGame);
}

/// Ends a round
void end_of_round(game_data *theGame)
{
    if (theGame->winner->locality == CPU)
    {
        alert("* * * * * * LOSER! * * * * * *");
        alert("Dealer has won the game with the hand:");
        show_hand(theGame->winner);
        
        alert("Your hand was:");
        show_hand(&theGame->players[1]);
    }
    else
    {
        alert("* * * * * * WINNER! * * * * * *");
        
        alert("The dealer had the following hand:");
        show_hand(&theGame->players[0]);
    }
    
    alert("======================");
    alert(" Starting a new round ");
    alert("======================");
    
    setup_game(theGame);
}

/// Main Program
int main()
{
    // Seed random based on current epoch
    srand((int)time(NULL));
    
    printf("    --   BLACKJACK v1.0   --\n");
    printf("    Written By Alex Cummaudo\n\n");
    
    game_data *theGame = (game_data*)malloc(sizeof(game_data));
    setup_game(theGame);
    
    // While we don't want to quit
    while (!theGame->quit)
    {
        // Given there is no winner
        if (!is_winner(theGame))
        {
            perform_round(theGame);
        }
        else
        {
            end_of_round(theGame);
        }
    }
    
    alert("Goodbye");
    return 0;
}