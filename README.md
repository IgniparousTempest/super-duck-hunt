# super-duck-hunt
A rewrite of the NES game "Duck Hunt" in C++ and SDL2

## Notes on original game

### Score

| Duck          | Rounds 1 - 5 | Rounds 6 - 10 | Rounds 11 - 15 | Rounds 16 - 20 | Rounds 21 - 99 |
|---------------|--------------|---------------|----------------|----------------|----------------|
| Black         | 500          | 800           | 1000           | 1000           | 1000           |
| Blue          | 1000         | 1600          | 2000           | 2000           | 2000           |
| Red           | 1500         | 2400          | 3000           | 3000           | 3000           |
| Perfect Bonus | 10000        | 10000         | 15000          | 20000          | 30000          |

### Successful shots needed

| Rounds        | Ducks Needed |
|---------------|--------------|
| 1 through 10  | 6 out of 10  |
| 11 through 12 | 7 out of 10  |
| 13 through 14 | 8 out of 10  |
| 15 through 19 | 9 out of 10  |
| 20 through 99 | 10 out of 10 |
| 0 onwards     | 10 out of 10 |

### Timing

#### Duck Speed

#### Dog Speed

#### UI Ducks Flash Speed