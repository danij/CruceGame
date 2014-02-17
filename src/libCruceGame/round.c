#include "round.h"
#include "errors.h"

#include <stdlib.h>
#include <stdio.h>

struct Round *round_createRound()
{
    struct Round *round = malloc(sizeof(struct Round));

    if (round == NULL)
        return NULL;

    for (int i = 0; i < MAX_HANDS; i++)
        round->hands[i] = NULL;

    for (int i = 0; i < MAX_GAME_PLAYERS; i++) {
        round->bids[i] = 0;
        round->players[i] = 0;
    }

    for (int i = 0; i < MAX_GAME_PLAYERS; i++)
        round->pointsNumber[i] = 0;

    return round;
}

int round_deleteRound(struct Round **round)
{
    if (round == NULL)
        return POINTER_NULL;
    if (*round == NULL)
        return ROUND_NULL;

    free(*round);
    *round = NULL;

    return NO_ERROR;
}

struct Hand *round_createHand()
{
    struct Hand *hand = malloc(sizeof(struct Hand));
    if (hand == NULL)
        return NULL;

    for (int i = 0; i < MAX_GAME_PLAYERS; i++) {
        hand->cards[i]   = NULL;
        hand->players[i] = NULL;
    }

    return hand;
}

int round_deleteHand(struct Hand **hand)
{
    if (hand == NULL)
        return POINTER_NULL;
    if (*hand == NULL)
        return HAND_NULL;

    free(*hand);
    *hand = NULL;

    return NO_ERROR;
}

struct Player *round_getBidWinner(const struct Round *round)
{
    if (round == NULL)
        return NULL;

    int maxBidIndex = 0;
    for (int i = 0; i < MAX_GAME_PLAYERS; i++) {
        if (round->players[i] != NULL &&
            round->bids[i] > round->bids[maxBidIndex]) {
            maxBidIndex = i;
        }
    }
    return round->players[maxBidIndex];
}

/**
 * @brief Helper to find player in a round.
 *
 * @param player Player to find.
 * @param round Round to search for player.
 * @return 0 on success, non-zero on failure.
 */
int findPlayerIndexRound(const struct Player *player, const struct Round *round)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (round == NULL)
        return ROUND_NULL;

    int i = 0;
    while (round->players[i] != player && 
           i < MAX_GAME_PLAYERS)
        i++;

    if (i == MAX_GAME_PLAYERS)
        return NOT_FOUND;

    return i;
}

int round_placeBid(const struct Player *player, int bid, struct Round *round)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (round == NULL)
        return ROUND_NULL;
    if (bid < 0)
        return ILLEGAL_VALUE;

    int index = findPlayerIndexRound(player, round);;

    if(index < 0)
        return NOT_FOUND;

    round->bids[index] = bid;

    return NO_ERROR;
}

int round_addPlayer(struct Player *player, struct Round *round)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (round == NULL)
        return ROUND_NULL;

    int index = findPlayerIndexRound(player, round);
    if(index >= 0) //impossible to add same player multiple time
        return DUPLICATE;

    int i = 0;
    while(round->players[i] != NULL && i < MAX_GAME_PLAYERS)
        i++;

    if (i == MAX_GAME_PLAYERS)
        return FULL;

    round->players[i] = player;

    return NO_ERROR;
}

int round_addPlayerHand(struct Player *player, struct Hand *hand)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (hand == NULL)
        return HAND_NULL;

    for (int i = 0; i < MAX_GAME_PLAYERS; i++)
        if (hand->players[i] == player)
            return DUPLICATE;

    int i = 0;
    while(hand->players[i] != NULL && i < MAX_GAME_PLAYERS)
        i++;

    if (i == MAX_GAME_PLAYERS)
        return FULL;

    hand->players[i] = player;

    return NO_ERROR;
}

int round_removePlayer(struct Player *player, struct Round *round)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (round == NULL)
        return ROUND_NULL;

    int index = findPlayerIndexRound(player, round);

    if (index < 0)
        return NOT_FOUND;

    round->players[index] = NULL;

    return NO_ERROR;
}

int round_removePlayerHand(struct Player *player, struct Hand *hand)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (hand == NULL)
        return HAND_NULL;

    for (int i = 0; i < MAX_GAME_PLAYERS; i++)
        if (hand->players[i] == player) {
            hand->players[i] = NULL;
            return NO_ERROR;
        }

    return NOT_FOUND;
}

int round_putCard(struct Player *player, int cardId,struct Hand *hand)
{
    if (player == NULL)
        return PLAYER_NULL;
    if (player->hand[cardId] == NULL)
        return CARD_NULL;
    if (hand == NULL)
        return HAND_NULL;

    for (int i = 0; i < MAX_GAME_PLAYERS; i++) {
        if (hand->players[i] == player){
            hand->cards[i] = player->hand[cardId];
            player->hand[cardId] = NULL;
            return NO_ERROR;
        }
    }

    return NOT_FOUND;
}

int round_computeScore(const struct Hand *hand)
{
    if (hand == NULL)
        return HAND_NULL;

    int cardsScore = 0;
    for (int i = 0; hand->players[i] != NULL; i++) {
        if (hand->cards[i] == NULL)
            return CARD_NULL;
        cardsScore += hand->cards[i]->value;
    }

    int gameScore = cardsScore / 33;

    return gameScore;
}

int totalPointsNumber(const struct Hand *hand)
{
    if (hand == NULL)
        return HAND_NULL;

    int points = 0;
    for (int i = 0; i < MAX_GAME_PLAYERS; i++)
        if (hand->players[i] != NULL && hand->cards[i] != NULL)
            points += hand->cards[i]->value;

    return points;
}

struct Player *round_handWinner(const struct Hand *hand, enum Suit trump,
                                struct Round *round)
{
    if (hand == NULL || trump == SuitEnd || round == NULL)
        return NULL;

    int playerWinner = -1;
    int numberPlayers = 0;
    for (int i = 0; i < MAX_GAME_PLAYERS; i++) {
        if ((hand->players[i] != NULL && hand->cards[i] == NULL) ||
            (hand->players[i] == NULL && hand->cards[i] != NULL))
            return NULL;
        if (hand->players[i] != NULL && hand->cards[i] != NULL) {
            numberPlayers++;
            if (playerWinner == -1)
                playerWinner = i;
        }
    }
    if (playerWinner == -1 || numberPlayers == 1)
        return NULL;

    int cardWinner;
    for (int i = playerWinner + 1; i < MAX_GAME_PLAYERS; i++) {
        if (hand->players[i] != NULL && hand->cards[i] != NULL) {
            cardWinner = deck_compareCards(hand->cards[playerWinner],
                                               hand->cards[i], trump);
            if (cardWinner == 2)
                playerWinner = i;
            if (cardWinner <= 0)
                return NULL;
        }
    }

    round->pointsNumber[playerWinner] += totalPointsNumber(hand);
    return hand->players[playerWinner];
}

int round_distributeCard(struct Deck *deck, const struct Round *round)
{
    if (deck == NULL)
        return DECK_NULL;
    if (round == NULL)
        return ROUND_NULL;

    int i, j;
    int distributedCards = 0;
    for (i = 0, j = 0; i < MAX_GAME_PLAYERS && j < DECK_SIZE; i++, j++) {
        //do not change while's order
        while (deck->cards[j] == NULL && j < DECK_SIZE)
            j++;
        while (round->players[i] == NULL && i < MAX_GAME_PLAYERS)
            i++;
        if (i < MAX_GAME_PLAYERS && j < DECK_SIZE) {
            int checkError = team_addCard(round->players[i], deck->cards[j]);
            if (checkError != NO_ERROR)
                return checkError;
            deck->cards[j] = NULL;
            distributedCards++;
        }
    }

    if (distributedCards == 0 && j == DECK_SIZE + 1)
        return DECK_EMPTY;
    if (distributedCards == 0 && i == MAX_GAME_PLAYERS + 1)
        return ROUND_EMPTY;
    if (distributedCards == 1 && i == MAX_GAME_PLAYERS + 1)
        return LESS_PLAYERS;
    if (distributedCards == 1 && j == DECK_SIZE + 1)
        return LESS_CARDS;

    return NO_ERROR;
}

int round_distributeDeck(struct Deck *deck, const struct Round *round)
{
    if (deck == NULL)
        return DECK_NULL;
    if (round == NULL)
        return ROUND_NULL;

    int numberPlayers = 0;
    for (int i = 0; i < MAX_GAME_PLAYERS; i++)
        if (round->players[i] != NULL)
            numberPlayers++;

    if (numberPlayers == 1)
        return LESS_PLAYERS;
    if (numberPlayers == 0)
        return ROUND_EMPTY;

    for (int i = 0; i < MAX_HANDS && i < DECK_SIZE / numberPlayers; i++) {
        int distributeCard = round_distributeCard(deck, round);
        if (distributeCard != NO_ERROR)
            return distributeCard;
    }

    return NO_ERROR;
}