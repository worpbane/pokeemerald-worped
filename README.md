# Pokémon Modern Emerald – Worped v0.1

This is a personal fork of Resetes12's Modern Emerald ROM hack.

The goal isn't to create a huge overhaul, but to tweak Emerald. I enjoy Modern Emerald as a base, and most of the changes are further quality-of-life improvements, UI tweaks, and adding Pokémon from later generations that I like.

I still have my original GBA copies of Sapphire and Emerald, and Hoenn has always been my favorite region. This project is mostly me experimenting with the pokeemerald codebase while building the version of Emerald I’d want to replay.

---

## Features

### 🧬 Pokémon Content
* Added Pokémon from later generations *(work in progress)*  
  - Lechonk (Gen IX)  
  - Stufful (Gen VII)  
  - *(Currently using Gen III movesets until proper ones are implemented.)*  
  
---

### 🧩 Gameplay & Systems
* **Expanded Starter Selection**  
  Based on: [TeamAqua Commit](https://github.com/pret/pokeemerald/commit/6d85a975dc561a24a5837d2992a03ef908870e6f)  
  Guide by [Archie and Mudskip](https://github.com/pret/pokeemerald/wiki/New-Birch's-Briefcase-With-Fully-Custom-Starters-by-Archie-and-Mudskip)  
  *(Birch now has starters from Generations I–III.)*

* **Catch Mode Toggle**  
  Based on: [FlashLucky's Repo](https://github.com/Flash1Lucky/pokeemerald-expansion/tree/catch-mode-toggle)  
  *(Backported from pokeemerald-expansion and modified to work with this project.)*

* **DexNav**  
  Based on: [ghoulslash's repo](https://github.com/ghoulslash/pokeemerald/tree/dexnav)
  
* **Seasons**
  Based on: [BelialClover's Commit](https://github.com/pret/pokeemerald/commit/1cdd1d0a877fc2e0929f84a33b6d9e4045102825)  
  *(Seasons change weekly, affect overworld visuals only (no gameplay impact), and can be advanced via the GameCube. Toggle available in the Start Menu.)*

---

### 🎮 UI Changes
* Move Info changed from **Start → L**

* **Gen 5–style Party Menu**  
  Based on: [Team Aqua's Repo](https://github.com/TeamAquasHideout/pokeemerald/tree/gen5ish_party_menu)

* **Black/White Summary Screen**  
  Based on:  
  - [RavePossum's Pokeemerald Vanilla Repo](https://github.com/ravepossum/pokeemerald/tree/bw_summary_screen)
  - [RavePossum's Pokeemerald Expansion Repo](https://github.com/ravepossum/pokeemerald-expansion/tree/bw_summary_screen_expansion)   
  *(Some features were manually backported from the pokeemerald-expansion version.)*  

---

## Planned Features

### 🧬 Pokémon Content
* Emolga 
* Mimikyu  
* Minccino
* Hatenna → Hattrem → Hatterene  
* Buneary → Lopunny  
* Paldean Wooper → Clodsire  

### Other Improvements
* Type icons next to Pokémon during battle  
  *(Attempting to backport the feature from pokeemerald-expansion, but the implementation differs significantly and may not be feasible with my current skill level...)*

---

## Notes

Some parts of the code are messy! This is my first time working on a project like this. Most of my coding experience has been C#, ActionScript 3, HTML, and CSS.  

I'm mainly uploading this in case anyone finds it useful or interesting.

---

## Compiling Pokémon Modern Emerald - Worped

For full build instructions, see [Pret's guide on building the rom](https://github.com/pret/pokeemerald/blob/master/INSTALL.md).  
Like Modern Emerald, use "make modern".

---

## Credits & Resources

This project makes use of various community repositories and guides:

- pokeemerald (pret)  
- pokeemerald-expansion  
- Team Aqua’s repository  
- RavePossum’s repositories  
- Archie and Mudskip’s starter selection guide  
