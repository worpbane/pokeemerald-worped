# Pokémon Modern Emerald – Worped v0.2 (wip)

This is a personal fork of [Resetes12's Modern Emerald ROM hack](https://github.com/resetes12/pokeemerald).

**🧪 Currently Testing On:** New Nintendo 3DS w/ open_agb_firm

---

The goal isn’t to create a massive overhaul, but to tweak Emerald in ways I personally enjoy. This includes quality-of-life improvements, UI refinements, and adding Pokémon from later generations that I like.  

One feature I’ve really focused on is shiny hunting, since I love Shinies. This modification actually began because I wanted to add DexNav to make finding them easier, and it just kind of grew from there.  

I still have my original GBA carts of Sapphire and Emerald, and Hoenn has always been my favorite region. This project is mostly me experimenting with the `pokeemerald` codebase while building the version of Emerald I’d like to replay.

---

## 🧬 Pokémon & Encounters

I didn't want to add 500 new mon, just Pokémon I actually like. Most of these are here because I think they're cute and wanted to see them in Hoenn.

| Pokémon | Origin | Where to find (Base Form) |
| :--- | :--- | :--- |
| **Buneary** → Lopunny | Gen IV | Routes 104 & 117 |
| **Emolga** | Gen V | Routes 110 & 116 |
| **Minccino** → Cinccino | Gen V | Petalburg Woods & Route 110 |
| **Stufful** → Bewear | Gen VII | Route 123 |
| **Mimikyu** | Gen VII | Mt. Pyre (Exterior & Summit) |
| **Hatenna** → Hattrem → Hatterene | Gen VIII | Routes 114 & 117 |
| **Snom** → Frosmoth | Gen VIII | Shoal Cave - Ice Room |
| **Lechonk** → Oinkologne(M/F) | Gen IX | Routes 101 & 102 |
| **Paldean Wooper** → Clodsire | Gen IX | Route 113 |
  
* **Mimikyu Note:** Disguised form only. It's a rare find at Mt. Pyre right now. I’m planning to make it night-only once I get the scripts sorted.
* **Movesets:** A mix of Gen I–III, Modern Emerald, and four backported moves.
* **Sprites:** Sourced from the [pokeemerald-expansion repo](https://github.com/rh-hideout/pokeemerald-expansion).  

### ⚡ Move Additions
I mostly added these because I wanted the **Hatenna line** to actually have some STAB moves, and because **Minccino** deserves Tail Whip.

* **Sprinkled through Pokémon who can learn it in later games** Dazzling Gleam, Draining Kiss, and Disarming Voice. I was having issues adding the TMs so they're disabled for now.
* **Exclusive:** Tail Slap for Minccino.

### 🌸 PokéBall Additions

I wanted to add some more pastel options for PokéBalls, kinda matches the idea of adding cute Pokémon.

* **Dream Ball:**  
	*4x Catch Rate when the wild Pokémon is sleeping.  
	* **Acquisition:** Obtained once a day from an old lady in Lilycove City after showing her a sleeping Pokémon.  
* **Heal Ball:**  
	*Heals captured PokéMon when they're added to the player's party. 
	* **Acquisition:** Sold at the Lavaridge Town PokéMart (near the healing hot springs).
* **Love Ball**:
	*8x Catch Rate when wild PokéMon is the same species but different gender as the player's attacking PokéMon.
	* **Acquisition:** Sold at the Verdanturf Town PokéMart (near the "Tunnel of Love").

---

## 🧩 Gameplay & Systems

- **Catch Mode Toggle**  
	Backported because it looked cool, and since my whole desire was capturing cool Pokémon it just made sense.  
	Based on [FlashLucky's repo](https://github.com/Flash1Lucky/pokeemerald-expansion/tree/catch-mode-toggle)  

- **DexNav**  
	This was the main purpose of this fork. I've redone the DexNav menu and how the system works. I disabled Search Levels to save memory, but buffed the Chain Bonuses to make finding shinies a bit easier. Your chances get significantly better much earlier than regular DexNav.  
	Based on [ghoulslash's repo](https://github.com/ghoulslash/pokeemerald/tree/dexnav)  
	*(Search Levels disabled to save save space. Chain Bonuses have buffed to assist shiny hunting. Received when you get the PokéNav. **Press L** when the start menu cursor is over the PokéNav.)*  

- **Expanded Starter Selection**  
	This was added because it sounded cool. Birch offers the starters from Generations I-III.  
	Based on [TeamAqua Commit](https://github.com/pret/pokeemerald/commit/6d85a975dc561a24a5837d2992a03ef908870e6f)  
	Guide by [Archie and Mudskip](https://github.com/pret/pokeemerald/wiki/New-Birch's-Briefcase-With-Fully-Custom-Starters-by-Archie-and-Mudskip)  

- **Shiny Charm**  
	Added a Shiny Charm to aid with shiny hunting. If you choose the lower shiny levels in Modern Emerald's starting options, it gives you like a 75% chance for shinies. It's ridiculous and I love it and you cannot toggle it.  
	Sprite sourced from [pokeemerald-expansion repo](https://github.com/rh-hideout/pokeemerald-expansion)  
	*(Received from the girl next to Mr. Briney's Cottage after earning your first Gym Badge in Rustboro.)*  

- **Seasons**  
	Pretty colors. I really liked the fall recolors of Hoenn, but didn't want it all the time. There aren't seasonal encounters of any sort, just the colors change every week.  
	Based on [BelialClover's Commit](https://github.com/pret/pokeemerald/commit/1cdd1d0a877fc2e0929f84a33b6d9e4045102825) & [Emerald Enhanced](https://github.com/Enhanced-Projects/Emerald-Enhanced)  
	*(Seasons change weekly, affect overworld visuals only, can be advanced via the GameCube, and are toggleable in the Start Menu.)*  

---

## 🎮 UI & Visuals 

#### ⚔️ Improved Battle Interface
The Battle Interface now provides **comprehensive data at a glance**, inspired by modern standards seen in `pokeemerald-expansion` and FireRed hacks like *Radical Red* and *Unbound*. 

* **Purple Health Boxes:**  
	* Features a custom Lavender and Silver theme. Because I like purple.
* **Move Info Pane Update**: Displays Move Type Icon, PSS Category, Remaining PP, Effectiveness, and STAB status.  
	* **Effectiveness Icons**: Dynamic indicators for *Super Effective* (Up Arrow), *Not Very Effective* (Down Arrow), and *No Effect* (X).  
	* **STAB Icon**: A contextual blue **S** appears if the selected move provides a Same-Type Attack Bonus.  
* **Dynamic Pokémon Type Indicators:**  
	* Small type icons now appear on health boxes during action selection. (Backported from `pokeemerald-expansion` and 'optimized' to work with `pokeemerald`).
* **Unified Type Icons**: 
    * Completely redrew all 18 Type Icons specifically for this interface.  
	* **3 Sizes:** Slim, used in Summary Screen, PokéDex, and DexNav. Battle, used in the Move Info Pane. Battle Mini, used for the Pokémon Type Indicators. Probably could have done this better.  
    * **Hybrid Design**: Merges classic high-visibility color blocks with modern stylized iconography.  
    * **Optimization**: All 18 icons use a single palette!  
	
* **Notes:**
I'm not going to lie, the implementation for these is probably really bad, I made the move info pane by studying the logic within `battle_controller_player.c` and just went from there. The Pokémon Type Indicator next to the health boxes was made by copying the code from `pokeemerald-expansion` over, updating(downgrading?) the helpers it adds to stuff I found within `pokeemerald`, and then fixing any compile issues that came up. They look pretty and I love them.  

### ✨ Menu Changes

* **Modernized FRLG-style Summary Screen**  
	* Custom layout inspired by Pokémon FRLG, Gen V, and Pokémon Ocean Blue.  
	* **Animated Background.**  
	* **Expanded Skills:** Pokémon Skills screen shows IVs, EVs, and Base Stats. Nature Indicators also appear.  
	* **Characteristics:** Integrated Gen IV+ personality traits into the Trainer Memo!  
	* Built on the foundation of [RavePossum's Pokeemerald Vanilla Repo](https://github.com/ravepossum/pokeemerald/tree/bw_summary_screen) & some bits of code backported from his [Expansion Repo](https://github.com/ravepossum/pokeemerald-expansion/tree/bw_summary_screen_expansion).  
	
* **Custom PokéDex Screen**  
	* Features a ground-up redesign inspired by the Summary Screen I worked on, creating a seamless aesthetic transition between menus!  
	* **Information:** I tuned down the amount of information given on the Stats screen, keeping what I thought was important.  
	* **Dex Modes:** Full support for Hoenn, National, and Search modes. The UI intelligently adapts its color palette and list headers based on the active mode.  
	* **PokéMon Numbering:** Caught Hoenn species show their regional number, while foreign species default to their National Dex entry.  
	* Built on the foundation of [TheXaman's Pokeemerald Vanilla Repo](https://github.com/TheXaman/pokeemerald/tree/tx_pokedexPlus_hgss).    

* **Gen 5–style Party Menu**  
	I thought this matched the BW Summary Screen and wanted to throw it in.  
	Based on [Team Aqua's Repo](https://github.com/TeamAquasHideout/pokeemerald/tree/gen5ish_party_menu)  
	
* **New Main Menu UI**  
	* Archie and Mudskip's New Main Menu UI was so nice looking I really wanted to implement it!
	* Changed the mugshot pictures to the trainer sprites.  
	* Changed color to purple.  
	* Based on [Mudskip's guide](https://github.com/pret/pokeemerald/wiki/New-Main-Menu-UI-With-Mugshot-by-Archie-and-Mudskip)  
    
### 🛠️ Small Tweaks & Fixes  

- **Move Info Shortcut:**  
	Changed from **Start → L**. This was just a bit more comfortable to use during battle on a handheld.
	
- **Second Select Item Removed:**  
	The ability to map a Bag item to holding/tapping Select didn't work at all for me on the 3DS or Steam Deck(mGBA), so I removed it as it was more of an annoyance. So far this has not had any negative impact on gameplay for me.

- **Item Description Headers**  
	I saw these in a few romhacks and liked the utility of seeing what an item does, mainly for berries since I can't keep track of them all. Ran into some issues with the berries not displaying right, so currently berries ALWAYS show their header, not just for the first time. But I kind of like how it looks and might just have it ALWAYS show headers.  
	Based on [ghoulslash's repo](https://github.com/ghoulslash/pokeemerald/tree/item_desc_header)  

- **Plural Give Item Fix**  
	Just small QOL. Saw it when I was looking into the Description Headers and threw it in.  
	Based on [ghoulslash's repo](https://github.com/ghoulslash/pokeemerald/tree/plural-giveitem)

---

## 🚀 Planned Features

### 🧬 Pokémon Content
- Iron Valient
- Goomy > Sliggo > Goodra
- Tinkatink > Tinkastuff > Tinkaton
- Morpeko
- Pachirisu

### Other Improvements
- **Night Encounters:** Specifically so Mimikyu only appears at night like it's supposed to.  
- **Ability Popups in Battle:** I'm going to try and backport this feature from `pokeemerald-expansion`...  
- **Update PokéDex Visuals:** I'd like to look over the PokéDex and see if I can't spice it up(make it purple).

---

## 📝 Notes

Some parts of the code are messy! This is my first time working on a project like this. Most of my coding experience has been in C#, ActionScript 3, HTML, and CSS, so jumping into C has been an adventure.

I'm mainly uploading this in case anyone finds it useful or interesting for their own forks.

---

## 🛠️ Compiling Pokémon Modern Emerald - Worped

For full build instructions, see [Pret's guide on building the rom](https://github.com/pret/pokeemerald/blob/master/INSTALL.md).  
Like the original Modern Emerald, use **"make modern"** to build.

---

## 🤝 Credits & Resources

This project wouldn't exist without these community repositories and guides:

- **pokeemerald** (pret)  
- **pokeemerald-expansion** (rh-hideout)  
- **Team Aqua’s** repository  
- **RavePossum’s** repositories  
- **Archie and Mudskip’s** starter selection guide  
- **ghoulslash’s** repositories  

### 🤖 A Small Note on AI
I wrote this README myself, but I used an AI (Gemini) to help me clean up the formatting, fix grammar, and make the table look nice.  
All the coding and art modifications have been me.... Sadly. Adding Pokémon is painful for regular `pokeemerald`(`pokeemerald-expansion` has a really nice looking JSON file...). But this was worth it to have my cute little Lechonk in my favorite game.
