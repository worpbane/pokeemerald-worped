# Pokémon Modern Emerald – Worped

## [0.2] - In Development

**🧪 Testing On:** New Nintendo 3DS w/ open_agb_firm

---

### ✨ Pokémon Additions
- None yet

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

---

### 🎮 UI Changes
- Start Menu reordered for improved usability.
- DexNav removed from Start Menu; now accessible by pressing **L** when the cursor is over the PokéNav.
- Implemented Item Description Headers from ghoulslash.
- Implemented Plural Give Item Fix from ghoulslash.

---

### 🎨 Visual Updates
- DexNav screen completely redone to reflect chain-based shiny mechanics.

---

### 📦 Asset Sources
- Shiny Charm implemented using the RHH Pokémon Emerald Expansion repository.

---

### 🐞 Bug Fixes
- Shiny Pokemon now display properly in the Summary Screen.
- Catch Mode should work more reliably. Moved it into the Adjust Damage step instead of Damage Calc. 
- DexNav no longer crashes in caves.

### ⚠️ Known Issues
- Seasonal palette changes currently do not affect tall grass.   
- Opening the Summary Screen from the PC, and then closing it, glitches the PC background.  
- Item Description Headers don't display correctly for berries, temporary fix is the headers ALWAYS display for berries.  
- Bike Switch button is same as the DexNav search... Need to figure out a way to disable dexnav search when on bike.

## [0.1] - Released 03/23/26

---

### ✨ Pokémon Additions

#### Gen IV
- [ ] Buneary  
- [ ] Lopunny  

#### Gen VII
- [x] Stufful  
- [ ] Mimikyu  

#### Gen VIII
- [ ] Hatenna  
- [ ] Hattrem  
- [ ] Hatterene  

#### Gen IX
- [x] Lechonk  
- [ ] Wooper (Paldean)  
- [ ] Clodsire  

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