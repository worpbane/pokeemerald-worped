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
- Add TMHM learnsets and Tutor learnsets

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
  - **Tail Whip**
	- Added **TMs** for the 3 new **Fairy-type** moves:  
	*(Can be purchased at TMs Shops across Hoenn)*   
	- Prepared contest/battle backend for these moves.  
	- Moves added to some Pokémon in the level_up_learnsets and tmhm_learnsets.

---

### 🎮 UI Changes
- New Main Menu UI
- Start Menu reordered for improved usability.
- DexNav removed from Start Menu; now accessible by pressing **L** when the cursor is over the PokéNav.
- Implemented Item Description Headers from ghoulslash.
- Implemented Plural Give Item Fix from ghoulslash.

---

### 🎨 Visual Updates
- DexNav screen completely redone to reflect chain-based shiny mechanics.  
- Added HexManiac sprite from the Team Aqua Hideout Repo.  
- Overworld follower sprites added for new Pokémon

### 📦 Asset Sources
- Shiny Charm implemented using the RHH Pokémon Emerald Expansion repository.

---

### 🐞 Bug Fixes
- Shiny Pokémon now display properly in the Summary Screen.
- Catch Mode should work more reliably. Moved it into the Adjust Damage step instead of Damage Calc. 
- DexNav no longer crashes in caves.

### ⚠️ Known Issues
- Seasonal palette changes currently do not affect tall grass.   
- Opening the Summary Screen from the PC, and then closing it, glitches the PC background.  
- Pokémon whose overworld sprites use the TRACKS_SLITHER value crash the game when making paths. This seems to be a Modern Emerald issue I inherited. TRACKS_BIKE_TIRE works fine though, and I'm going to compare the two and see where things aren't matching.
- Summary Screen isn't showing Status on moves, only Phsyical and Special.

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