# DIY-3axis-CNC-PCB-plotter-on-uln2003
DIY simple and cheap PCB plotter for making PCBs with etching method and just plotter for printing simple arts orr maybe text and homework for school)
This repository contains the complete documentation, source code, and design specifications for a low-cost, high-precision three-axis CNC plotter designed to automate the process of drawing circuit board traces directly onto copper-clad laminates using permanent ink prior to chemical etching.

The structural mechanics of this project are based on the design by maker101io (MakerWorld), with a major engineering modification implemented on the vertical Z-axis. The standard hobby servo mechanism was replaced with a rigid rack-and-pinion assembly driven by a dedicated stepper motor, adapted from the BadgerJed design on Thingiverse (Thing 4672712).

---
### Video Demonstration
[Watch the CNC Plotter in action](Media/5190482863572692207.mp4)
## Bill of Materials (BOM) and Cost Estimation

The system relies on accessible, non-proprietary components commonly found in entry-level automation kits, eliminating the need for expensive dedicated stepper drivers (such as A4988 or DRV8825).

| Item Description | Qty | Functional Purpose | Estimated Cost (USD) |
| :--- | :---: | :--- | :--- |
| **Arduino Nano** | 1 | Main microcontroller unit (ATmega328P) | ~$3.50 |
| **28BYJ-48 Stepper Motor (5V)** | 3 | Actuators for X, Y, and modified Z axes | ~$5.00 (Total) |
| **ULN2003 Driver Board** | 3 | Darlington transistor arrays for power switching | ~$2.50 (Total) |
| **Power Supply 12V (1.5A / 18W)** | 1 | High-voltage supply for motor coils | Coopted / ~$4.50 |
| **Solderless Breadboard & Jumpers**| 1 | Prototype circuit routing matrix | ~$3.00 |
| **DC Power Jack (Screw Terminals)**| 1 | Secure 12V interface connection | ~$0.50 |
| **3D Printing Filament (PLA/PETG)** | ~300g| Frame, rails, and carriage components | ~$4.00 |
| **Industrial Permanent Marker** | 1 | Acid-resistant etch-resist medium | ~$1.00 |
| **TOTAL ESTIMATED BUDGET:** | | | **~$24.00** |

---

## Electrical Interface and Wiring Topology

> ⚠️ **CRITICAL ELECTRICAL NOTICE:** The 12V power supply must be routed exclusively to the high-voltage rails supplying the ULN2003 driver boards. The Arduino Nano must be powered separately via its USB interface from the host computer. It is mandatory to establish a common ground by tying the Arduino GND pin to the negative terminal of the 12V supply rail on the breadboard.

* **X-Axis (Marker Carriage):** Arduino Digital Pins `D2, D3, D4, D5` ➡️ Inputs `IN1, IN2, IN3, IN4` of the primary ULN2003 board.
* **Y-Axis (Linear Bed):** Arduino Analog Pins `A0, A1, A2, A3` (configured as digital outputs) ➡️ Inputs `IN1, IN2, IN3, IN4` of the secondary ULN2003 board.
* **Z-Axis (Vertical Lift):** Arduino Digital Pins `D9, D10, D11, D12` ➡️ Inputs `IN1, IN2, IN3, IN4` of the tertiary ULN2003 board.

---

## Mechanical Assembly and Friction Mitigation

1. Fabricate the frame and linear bed components using the STL files from the **maker101io** project specifications.
2. Fabricate and install the rack-and-pinion Z-axis mechanism using the specifications from **Thingiverse project 4672712**. Ensure precise mesh tolerances between the circular pinion gear on the motor shaft and the linear rack of the marker holder.
3. **Friction Control Protocol:** Fused Deposition Modeling (FDM) parts exhibit surface roughness that causes mechanical binding. Loosen the carriage retention fasteners slightly to eliminate binding, then apply a layer of dry paraffin wax or solid dry soap directly to the guide rails and gear teeth. This solid lubricant reduces the friction coefficient significantly, preventing motor stall conditions under structural loads.

---

## Mathematical Calibration of Actuator Kinematics

The deployed firmware runs a custom G-code parsing algorithm written specifically to switch the phases of the ULN2003 arrays sequentially. The algorithm includes a thermal protection routine that de-energizes all motor coils upon completion of a linear command sequence, preventing inductive heating at 12V.

The verified linear resolution for both the X and Y axes is calculated and calibrated to exactly **`41.805` steps per millimeter**.

### Spatial Calibration Procedure:
If dimensional inaccuracies are detected, perform the following verification protocol:
1. Flash the firmware with the scaling constant set to temporary unity: `const float stepsPerMm = 1.0;`.
2. Issue the manual serial command `G1 X1000 F100` via the console to execute exactly 1000 steps.
3. Measure the absolute displacement with a digital caliper or precision rule (e.g., observed displacement = 23.92 mm).
4. Calculate the true scaling factor: `1000 / 23.92 = 41.806` and update the firmware constant.

---

## Computational Workflow: LaserGRBL Execution Parameters

To prevent spatial distortion, asymmetric scaling, and trace misalignment, the raster-to-vector translation process must follow these configuration steps:

### Phase 1: Vectorization Options
1. Launch LaserGRBL, select the designated COM port, set the baud rate to **`115200`**, and initialize communication via the connection icon.
2. Import the monochrome circuit layout image exported from EasyEDA.
3. Set the import method to **Vectorization**.
4. In the **Filling** parameter dropdown menu, select **None**. This configuration forces the compiler to trace the boundaries of the paths instead of executing high-frequency raster line scans, minimizing geometric artifacts caused by mechanical backlash.
5. Enable the **Optimize Travel** option to compute the shortest sequential toolpath, reducing idle displacement.

### Phase 2: Dimensional Scaling and Z-Axis Routing
1. In the subsequent configuration window, ensure that the **Keep Aspect Ratio (Lock Icon 🔒)** option is enabled.
2. Input the exact dimension of the target substrate into the **Width (W)** field (e.g., `70.0` mm). The **Height (H)** field will be derived programmatically, preserving geometric fidelity. Manual input in both fields can distort rectangular proportions into square geometries.
3. Set the **Speed Limit** to **`100`** or **`150`** mm/min. Low feed rates minimize systemic inertia and displacement errors.
4. Set the **Laser Mode** parameters to **`M3 - Constant Power`**. The deployed firmware intercepts the spindle speed constants `S0` (retract marker) and `S90` (deploy marker) within the runtime execution blocks.

### Phase 3: Etch-Resist Layer Density (Multi-Pass Mode)
Single-layer ink deposition leaves micro-porosities through which the chemical etchant will penetrate, degrading the underlying copper traces.
1. Locate the **Passes** parameter field in the lower-left quadrant of the configuration window.
2. Overwrite the default value and set it to **`2`** or **`3`**.
3. Compile the file by clicking **Create**. The software generates overlapping toolpaths, forcing the machine to re-trace the geometries multiple times sequentially. This results in a dense, non-porous barrier capable of resisting chemical solutions during the etching process.

---

## System Operation Protocol

1. Prepare the copper substrate by scrubbing the surface with an abrasive cleaner to remove oxidation, followed by an isopropyl alcohol or acetone rinse to remove surface lipids.
2. Secure the substrate to the machine bed using high-tack double-sided adhesive tape.
3. Manually align the marker tip with the designated lower-left margin of the copper plate.
4. Execute the serial command **`G92 X0 Y0`** to set the absolute software datum.
5. Energize the 12V power supply and select **Play** to begin execution.
