# DiceDuel
For [CodingGame's weekly challenge of 2021/10/18](https://www.codingame.com/multiplayer/bot-programming/dice-duel)

## Approach
### Simulation
First, I want to simulate the game, so I don't rely on CG's interface for every change I make. The simulation is not time limited, which is a difference that should 
be taken in account later.
The simulation should behave like the real game.

### Strategy
My idea is to simulate every move for every player for many turns in advance, in order to choose the best option ( aka opponent not winning ).
Currently, I don't know how to make it more efficient. I don't want to exclude the defensive options ( such as getting a die out of range ) so I have to simulate
every position for both me and my opponent. 
