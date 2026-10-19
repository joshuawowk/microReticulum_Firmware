# Research: Harbor Breeze 60 lm / 1 W solar spotlight (Lowe's 5018244381)

Collected 2026-10-19 by web research for the solar repeater build. Every claim has
a source; anything not tied to a source is marked **unverified**. Lowe's,
Printables, MakerWorld, Patreon, Hackster and YouTube pages returned HTTP 403 to
automated fetches, so several items rely on search-result summaries.

**Bottom line:** the cell and panel specifications for item 5018244381
specifically could not be verified. Community sources describe several
Harbor Breeze lights with *different* cells — including LiFePO4. **Do not
assume the stock cell is Li-ion. Open your unit and read the label.**

## Facts

| Fact | Value | Source | Confidence |
|---|---|---|---|
| Product exists | "Harbor Breeze 60-Lumens 1-Watt Black Warm White Solar LED Spot light" | [Lowe's](https://www.lowes.com/pd/Harbor-Breeze-60-Lumens-1-Watt-Black-Warm-White-Solar-LED-Spot-light/5018244381) (search snippet), [Shipt](https://www.shipt.com/shop/products/97744112-0837-11f1-ae1c-c7824574cbec) | Medium |
| Item # / Model # for 5018244381 | **Unverified** (page 403s) | — | None |
| Other item #s used in Meshtastic guides | 1234868 (Model ZDL1362), 5015598033, 1002689960 (SL1832, 0.6 W flood), 2483282 — none confirmed to be 5018244381 | [Philly Mesh 2026](https://phillymesh.net/2026/07/10/harbor-breeze-node-updated/), [Hackaday](https://hackaday.io/project/194509-harbor-breeze-mesh-node-hack), [Philly Mesh 2025](https://phillymesh.net/2025/06/21/harbor-breeze-node/) | Medium |
| OEM | Harbor Breeze is a Lowe's house brand; OEM not identified | — | Unverified |
| Price | Regular ≈ $15; sale $9.98 (July 2026); ≈ $9 each in 6+ quantities | [Slickdeals](https://slickdeals.net/f/19769490-harbor-breeze-solar-led-spotlight-30-lumens-matte-black-warm-white-4-60-lumens-9-98-free-store-pickup-at-lowe-s-or-free-shipping-on-35), [DealNews](https://www.dealnews.com/products/Harbor-Breeze/Harbor-Breeze-1-Watt-60-Lumen-Solar-LED-Spot-Light/498670.html), [Philly Mesh 2026](https://phillymesh.net/2026/07/10/harbor-breeze-node-updated/) | Medium-high (expired deals; today's price not visible) |
| IP rating | Not published for this SKU ("wet rated"). NodakMesh estimates ≈ IPX5 for the SL1832. | [NodakMesh](https://nodakmesh.org/blog/harbor-breeze-solar-node-assembly) | Low |
| Panel (SL1832 variant only) | ≈ 5–6 V, "about 0.5 W, ~90 mA at 5 V in full sun", ≈ 6 V open circuit | [NodakMesh](https://nodakmesh.org/blog/harbor-breeze-solar-node-assembly) | Low-medium (author estimate) |
| Panel for 5018244381 | **Unverified**; "1-Watt" is a nameplate figure. One commenter says the 60 lm model has a "bigger panel, bigger battery" than the 30 lm. | [Slickdeals](https://slickdeals.net/f/19769490) | Low |
| Charge-controller IC | **Not named in any source** | — | Unverified |
| Stock board behaviour | Slide switch + dusk/dawn controller. CSRA strips the light-control IC; Philly Mesh keeps the stock board. | [CSRA](https://meshcore.csramesh.org/diy-tree-node/), [Philly Mesh 2025](https://phillymesh.net/2025/06/21/harbor-breeze-node/) | Medium |
| Internal space | A RAK WisBlock base "fits perfectly inside the solar compartment"; RAK19003 is smaller | [Meshtastic docs](https://meshtastic.org/docs/community/enclosures/rak/harbor-breeze-solar-hack/) | Medium |

## Battery chemistry — what each source says

| Source | Light | Cell stated |
|---|---|---|
| [NodakMesh](https://nodakmesh.org/blog/harbor-breeze-solar-node-assembly) (Dec 2025, upd. May 2026) | SL1832 60 lm flood | Removable **18650 Li-ion, 3.7 V, 1500 mAh**; charger stops ≈ 4.2 V |
| Lowe's SL1832 listing (search snippet) | SL1832 | 3.7 V 1500 mAh 18650 |
| Lowe's SS86P-F60C-BK-T15 and SS86PA kit manual (search snippet) | 60 lm spotlight | **LiFePO4 18500, 3.2 V, 1000 mAh** |
| [CSRA MeshCore guide](https://meshcore.csramesh.org/diy-tree-node/) | "1-Watt Solar Security Light" ≈ $10 | "AA battery holder" — chemistry not given |
| [Philly Mesh 2026](https://phillymesh.net/2026/07/10/harbor-breeze-node-updated/) | Item 1234868 | New version has "a smaller battery"; chemistry not stated |
| [Hackaday](https://hackaday.io/project/194509-harbor-breeze-mesh-node-hack) | 5015598033 | "Lithium ion cell", 1500 mAh; a commenter pulled a **1.2 V** (over-discharged) cell from a dead node |

Production batches evidently differ. The 60 lm / 1 W spotlight may be the SS86P
family (LiFePO4) — that is an inference, not a verified fact.

## Existing build guides

| Guide | Board | Wiring | Antenna | Notes |
|---|---|---|---|---|
| [Meshtastic docs — Harbor Breeze solar hack](https://meshtastic.org/docs/community/enclosures/rak/harbor-breeze-solar-hack/) (tavdog) | RAK WisBlock | Battery → board battery port; panel → JST-ZH solar port | IPEX→SMA pigtail, drilled + siliconed | Prints: Thingiverse 6839973, 6849930; Printables 886187, 1347088, 1525190. LED housing less waterproof than panel compartment. |
| [Hackaday — Harbor Breeze Mesh Node Hack](https://hackaday.io/project/194509-harbor-breeze-mesh-node-hack) (2024-01-19) | RAK WisBlock | JST-PHR-2 on battery tabs; LED wires cut | SMA pigtail | Dead node had cell at 1.2 V because the panel was not on the WisBlock solar port |
| [NodakMesh](https://nodakmesh.org/blog/harbor-breeze-solar-node-assembly) (2025-12-18) | RAK4631 + RAK19007 | Stock panel → JST-ZH solar; 18650 → JST-PH battery; LED wires cut | SMA bulkhead | "Many days" without sun (no data). Condensation; 50–60 °C internal heat. |
| [Philly Mesh](https://phillymesh.net/2025/06/21/harbor-breeze-node/) (2025-06-21) | RAK4631 (XIAO tried) | JST-PHR-2 on stock board battery tabs; stock solar board kept | IPEX→SMA | **XIAO brownouts and boot loops.** Never power the RAK without an antenna. |
| [Philly Mesh updated](https://phillymesh.net/2026/07/10/harbor-breeze-node-updated/) (2026-07-10) | RAK4631 | JST into battery port; RAK polarity is reversed vs. wire colours | IPEX→SMA, GT-800 whip | ≈ 1 year of testing, "works just as well"; no runtime numbers |
| [CSRA MeshCore tree node](https://meshcore.csramesh.org/diy-tree-node/) (2026) | RAK19007 / RAK19003 | ZH pigtail (solar), PH pigtail (battery), stock switch inline | SMA pigtail ≈ 20 cm | Seal the weep hole |
| [Printables 1748950](https://www.printables.com/model/1748950) (2026) | Heltec T096 | not read | N-type, Alfa 915 MHz | YouTube series linked from the model |

Additional prints (not readable): Printables
[1225732](https://www.printables.com/model/1225732),
[1226442 (T114)](https://www.printables.com/model/1226442),
[1353528](https://www.printables.com/model/1353528),
[1611224 (T114)](https://www.printables.com/model/1611224),
[958722 (N-type)](https://www.printables.com/model/958722);
MakerWorld [1201132 (T114)](https://makerworld.com/en/models/1201132).

No guide found that measures autonomy or daily harvest. At the only quoted panel
figure (≈ 90 mA at 5 V, unverified) and 3–4 equivalent sun-hours, harvest is
roughly **270–360 mAh/day at best**; winter, tilt and shading cut that.

## Wiring guidance derived from this research

1. Open the unit first. Read the cell label; measure cell voltage, panel Voc in
   sun, and panel Isc. Photograph the controller board.
2. **Li-ion cell (3.7 V nominal, 4.2 V full):** feed the panel to a proper Li-ion
   charger (board's charger or the stock controller); never panel → cell directly.
3. **LiFePO4 cell (3.2 V nominal, 3.6 V full):** never charge it from a 4.2 V
   Li-ion charger (TP4056, XIAO BQ25101, RAK19007 solar input). Keep the stock
   controller (built for that cell) or use a 3.6 V LiFePO4 charger.
4. Cut the LED leads so the lamp cannot drain the cell at night.
5. Wire the node directly to the cell terminals, not the controller's switched
   load output (a likely cause of the reported XIAO brownouts).
6. Firmware must stop drawing current before the cell is damaged, and must come
   back on its own when the sun returns.
