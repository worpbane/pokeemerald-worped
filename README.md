# Pokémon Modern Emerald – Worped

This is a personal fork of Resetes12's Modern Emerald ROM hack.

The goal isn't to create a huge overhaul, but to tweak Emerald into a version I personally enjoy playing. Most of the changes are quality-of-life improvements, UI tweaks, and adding Pokémon from later generations that I like.

I still have my original GBA copies of Sapphire and Emerald, and Hoenn has always been my favorite region. This project is mostly me experimenting with the pokeemerald codebase while building the version of Emerald I’d want to replay.

## Features
* Added some Pokémon from later generations (work in progress)
	- Lechonk
	- Stufful
 	- *(Currently using Gen III movesets until I figure out how to add proper ones.)*
* Move Info changed from **Start → L**
* Catch Mode Toggle
  Based on: [FlashLucky's Repo](https://github.com/Flash1Lucky/pokeemerald-expansion/tree/catch-mode-toggle)  
  *(Backported from pokeemerald-expansion and modified to work with this project.)*
* **DexNav**
  Based on: [ghoulslash's repo](https://github.com/ghoulslash/pokeemerald/tree/dexnav)
* **Gen 5–style Party Menu**  
  Based on: [Team Aqua's Repo](https://github.com/TeamAquasHideout/pokeemerald/tree/gen5ish_party_menu)
* **Black/White Summary Screen**  
  Based on:
  * [RavePossum's Pokeemerald Vanilla Repo](https://github.com/ravepossum/pokeemerald/tree/bw_summary_screen)
  * [RavePossum's Pokeemerald Expansion Repo](https://github.com/ravepossum/pokeemerald-expansion/tree/bw_summary_screen_expansion)  
  *(Some features were manually backported from the pokeemerald-expansion version.)*

## Planned Features

Additional Pokémon (space permitting):

* Mimikyu
* Hatenna → Hattrem → Hatterene
* Buneary → Lopunny
* Paldean Wooper → Clodsire

UI Improvements:

* Type icons next to Pokémon during battle

## Notes

Some parts of the code are still messy — this is my first time working on a project like this.  
I'm mainly uploading it in case anyone finds it useful or interesting.

## Compiling Pokémon Modern Emerald - Worped

For full build instructions, see [Pret's guide on building the rom]([https://github.com/resetes12/pokeemerald](https://github.com/pret/pokeemerald/blob/master/INSTALL.md)).
Like Modern Emerald, use "make modern".
