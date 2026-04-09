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

* **New TMs:** Dazzling Gleam, Draining Kiss, and Disarming Voice.
* **Exclusive:** Tail Whip for Minccino.

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

### ✨ Menu Changes

- **Black/White-style Summary Screen**  
	Though I've never played GenV, I saw this and thought it looked awesome. The Gen III Summary Screen is just a lil dated.  
	Based on [RavePossum's Pokeemerald Vanilla Repo](https://github.com/ravepossum/pokeemerald/tree/bw_summary_screen) & [Expansion Repo](https://github.com/ravepossum/pokeemerald-expansion/tree/bw_summary_screen_expansion)  
	*(Some features were backported from the expansion version.)*  

- **Gen 5–style Party Menu**  
	I thought this matched the BW Summary Screen and wanted to throw it in.  
	Based on [Team Aqua's Repo](https://github.com/TeamAquasHideout/pokeemerald/tree/gen5ish_party_menu)  
	
- **New Main Menu UI**  
	Archie and Mudskip did a really nice job with this, I love that it shows your party and where you are. 10/10.  
	Based on [Mudskip's guide](https://github.com/pret/pokeemerald/wiki/New-Main-Menu-UI-With-Mugshot-by-Archie-and-Mudskip)  
	*(Changed color to purple, and I had to disable the function that hides the Mystery Gift boxes, as it was messing with the options menu. I DO NOT have the technical know how to fix. I know it has to do with rendering.)*  
    
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

### Other Improvements
- **Night Encounters:** Specifically so Mimikyu only appears at night like it's supposed to.

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
All the coding has been me.... Sadly. Adding Pokémon is painful for regular `pokeemerald`, `pokeemerald-expansion` has a really nice looking JSON file! But this was worth it to have my cute pig mon in my favorite game.
