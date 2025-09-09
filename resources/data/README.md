- This directory contains data packs for use with C&C Games. Currently there is only `td`, but `ra` will come later
- GitHub actions workflows copy the contents of these data packs into the same archive as the game binary
- Typically files from external patches etc. are only included here if they needed to be modified to work with the NCO engine or no easy source of download for specific files is available
- Files are listed for each data pack directory below

## `td`

- `sc-psx-n64.mix`: PS1 & N64 Extra Missions + N64 Video Files 
  - **Note**: This is a modified version of the files provided in Nyergud's C&C 1.06 patch, see [here](http://nyerguds.arsaneus-design.com/cnc95upd/cc95p106/patch106c_r3_en.html#toc_4) for credits of the original authors
  - Scenario IDs changed to work with the Covert Operations Menu logic
  - GDI Missions are `scg[51-56]ea`
  - NOD Missions are `scb[38-41]ea`
  - PATSUX mission is now Scenario No. `54` <!-- TODO: Add PATSUX mission number to RULES or amend scenario INI when aircraft rule overrides implemented -->
  - Note: PATSUX referenced in Source Code `scenario.cpp` to implement Laser ORCAs (for now)
- `snow.mix`, `snowicnh.mix`, `snowifre.mix` and `snowiger.mix`: The snow theater from Red Alert, converted to work with Tiberian Dawn
  - **Note**: These files are part of Nyergud's C&C 1.06 patch, see [here](http://nyerguds.arsaneus-design.com/cnc95upd/cc95p106/patch106c_r3_en.html#toc_4) for credits of the original authors (specifically `Jafet Kackur` for these files)
