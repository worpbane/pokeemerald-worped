# Pokémon Modern Emerald – Worped

## [0.2] - In Development (WIP)
**🧪 Testing On:** New Nintendo 3DS w/ open_agb_firm

---

### ✨ Pokémon Additions

#### Gen IV
- [x] Buneary  
- [x] Lopunny  

#### Gen V
- [x] Emolga  
- [x] Minccino  
- [x] Cinccino  

#### Gen VII
- [x] Bewear  
- [x] Mimikyu  

#### Gen VIII
- [x] Hatenna  
- [x] Hattrem  
- [x] Hatterene  
- [x] Snom  
- [x] Frosmoth  

#### Gen IX
- [x] Clodsire 
- [x] Oinkologne M 
- [x] Oinkologne F 
- [x] Wooper (Paldean)  

#### Wild Encounters
- Pokémon added to enviroments I thought fit them.

#### TODO
- Add TMHM learnsets and Tutor learnsets for new Pokémon.
- Add the three new moves to Tutor.

---

### 🧩 Features
- Added **Shiny Charm** to aid in finding rare Pokémon.  

- **DexNav updated**:
	- Search Level disabled (saves save block space).  
	- **Chains are everything now**: chaining skyrockets your shiny odds! The system scales dynamically based on your **chosen shiny rate**, so if you pick a wild low rate like 512, you’ll be practically **swimming** in shinies as you near a 50-chain streak. ⚡✨ 
    - DexNav will no longer even attempt to search for Pokémon if you are on a bike or in an area with no Pokémon.

- **Seasonal Weather**:
	- WIP, currently non-functional. Was going to go with Emerald Enhanced's implementation, but I'm not completely happy with that.  
	
- Removed **Hold Select** to register second item from the Bag.
	- The secondary (hold) register was unreliable in testing and would always trigger alongside the tap input.
	- Sometimes the tap input wouldn't even trigger for me.
	- Reverted Select button to standard behavior.
	
- Added new moves to the game:
  - **Dazzling Gleam**  
  - **Draining Kiss**  
  - **Disarming Voice**  
  - **Tail Slap**  
	- Prepared contest/battle backend for these moves.  
	- Moves added to some Pokémon in the level_up_learnsets.
	
- Added new PokéBalls to the game:  
	- **Dream Ball**  
	Works best on Sleeping Pokémon.  
	- **Heal Ball**  
	Heals Pokémon when captured and added to your party.  
	- **Love Ball**  
	8x catch rate on Pokémon of the same species but opposite gender.  
	Note: If updating from a previous version, please use the script in the Debug Menu to update your item IDs properly and prevent item corruption.

---

### 🎮 UI Changes & Battle Experience
- **Improved Battle Interface Move Info Pane**:
	- The move details pane now provides comprehensive data at a glance, inspired by FireRed hacks like Unbound, including a **Move Type Icon**, **PSS Icon**, **Remaining PP**, an **Effectiveness Icon**, and a **STAB Icon**.  
	- **Effectiveness Icons**: Dynamic indicators for Super Effective (Up Arrow), Not Very Effective (Down Arrow), and No Effect (X).  
	- **STAB Icon**: A contextual blue "S" appears if the selected move provides a Same-Type Attack Bonus.  
- **Improved Battle Interface Health Boxes**:
	- Added a nice Lavender theme to the boxes because I like purple.  
	- Type Icons now dynamically appear on health bars during action selection.
- **Unified Type Syetem**:  
	- Redrew all 18 Type Icons specifically for the new Battle Interface, added 'slim' versions for the Summary Screen/PokéDex/DexNav, and 'mini' versions to display next to Pokémon in battle. 
	- I'm calling them a Hybrid design, classic high-vis colors with the modern stylzed icons where I could fit them.  
	- Optimized for a single palette! I'm quite proud of this.  
- **Summary Screen Update**:
	- Updated the **BW Summary Screen** with new graphics!  
	- Changed to look more FRLG inspired.
	- Also includes **Characteristics** from later gens.
- **PokéDex Update**:
	- Updated the **HGSS PokéDex Plus Screen** with new graphics!  
	- Changed to look more FRLG Summary Screen inspired.  
	- Scaled back some information so it wasn't so cluttered.
- New **Main Menu UI**
- Start Menu reordered for improved usability.
- DexNav removed from Start Menu; now accessible by pressing **L** when the cursor is over the PokéNav.
- Implemented Item Description Headers from ghoulslash.
- Implemented Plural Give Item Fix from ghoulslash.

---

### 🎨 Visual Updates
- DexNav screen completely redone to reflect chain-based shiny mechanics.  
- Added **HexManiac** sprite (Team Aqua Hideout Repo).  
- Overworld follower sprites added for new Pokémon

### 📦 Asset Sources
- Shiny Charm implemented using the RHH Pokémon Emerald Expansion repository.

---

### 🐞 Bug Fixes
- Shiny Pokémon now display properly in the Summary Screen.
- Catch Mode should work more reliably. Moved it into the Adjust Damage step instead of Damage Calc. 
- DexNav no longer crashes in caves.
- Opening the Summary Screen from the Storage System no longer causes any graphical weirdness. It was caused by an incorrect call from when I marged the BW Summary Screen into Modern Emerald.

### ⚠️ Known Issues 
- Pokémon whose overworld sprites use the TRACKS_SLITHER value crash the game when making paths. This seems to be a Modern Emerald issue I inherited. TRACKS_BIKE_TIRE works fine though, and I'm going to compare the two and see where things aren't matching.
- Abandoned Ship Level 2 is bugged? This might be another Modern Emerald thing, not sure.

---

## [0.1] - Released 03/23/26

---

### ✨ Pokémon Additions

#### Gen VII
- [x] Stufful  

#### Gen IX
- [x] Lechonk  

---

### 🧩 Features
- Starter selection expanded to include 9 Pokémon, featuring all starters from **Gen I–III**
- Catch Mode toggle implemented in **Wild Battles**  
- DexNav implemented in areas with **Wild Pokémon**  
- **Season System**
  - Seasons change weekly and affect **overworld visuals only** (no gameplay impact)  
  - Can be toggled in the **Start Menu**  
  - Manually advance seasons by interacting with the **GameCube in the player's room**

---

### 🎮 UI Changes
- Move Info button changed from **Start → L**  
- Party Menu updated to resemble **Gen V**  
- Summary Screen updated to resemble **Gen V**  

---

### 🎨 Visual Updates
- Updated type indicators using community repositories.  
- Replaced several trainer sprites using assets from community repositories.  
- Overworld tileset palettes now change based on the current season.

---

### 📦 Asset Sources
- Pokémon content implemented using the RHH Pokémon Emerald Expansion repository.  
- Various UI elements and trainer sprites sourced from the Team Aqua repository.

---

### 🐞 Bug Fixes
- None yet  

### ⚠️ Known Issues
- Seasonal palette changes currently do not affect tall grass.