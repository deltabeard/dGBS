# dGBS

Decompiled Game Boy Sound.

Plays Game Boy audio using decompiled instructions that manipulate the audio registers of the Game Boy APU only.

## Usage

Requires minigbs from https://github.com/deltabeard/MiniGBS/tree/decoded

```
# Generate dgbs file in current folder. minigbs compiled from 'decoded' branch.
minigbs /run/media/mahyar/WD/Games/ROMs/GBS/pokeprism.gbs 1 600

# Play dgbs file.
dgbs_player dgbs
```
