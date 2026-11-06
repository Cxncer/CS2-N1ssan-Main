# N1SSAN — Complete System Architecture & Functional Responsibilities

## 1. Core Architecture

N1SSAN consists of two user-facing systems, one central launcher, one native runtime, and a web bridge.

The two user-facing systems are:

- **Loadout:** Equipment configuration.
- **Armory:** Inventory and item collection management.

The fundamental distinction is:

**Armory = What the player owns.**

**Loadout = What the player equips.**

These are separate systems with separate responsibilities, but they share the same underlying inventory data and native runtime.

---

## 2. Loadout — Equipment Management

Loadout is the player's equipment configuration interface.

Its purpose is to let the player choose which owned skins and items are assigned to their in-game equipment slots.

It organizes equipment into categories such as:

- Pistols
- Rifles
- SMGs
- Heavy
- Knives
- Gloves
- Agents
- Zeus

### Loadout responsibilities

- Display equipment slots and their currently assigned items.
- Allow the player to select an item for an equipment slot.
- Support team-specific equipment configurations where applicable.
- Display selected skin previews and relevant details.
- Save and manage equipment profiles.
- Apply the selected equipment configuration through the native DLL.

**Important:** Loadout is not the complete item collection. It focuses on equipment selection and assignment.

---

## 3. Armory — Inventory Management

Armory is the player's complete item collection and inventory management interface.

Its purpose is to display and manage all available items, whether equipped or unequipped.

### Armory responsibilities

- Display the complete item collection.
- Browse and organize skins across weapon and equipment categories.
- Display item details, including:
  - Skin name
  - Weapon/item type
  - Rarity and quality
  - Float value
  - Paint seed
  - StatTrak status
  - Stickers
  - Charms
  - Custom name
- Support inventory organization and item selection.
- Manage item-specific data and metadata.
- Provide available items for Loadout configuration.

**Important:** Armory does not decide what the player currently equips. It manages the collection from which Loadout can select items.

---

## 4. How Loadout and Armory Work Together

The relationship should work as follows:

1. Armory contains the player's available items.
2. Loadout reads available items from the shared inventory data.
3. The player selects an item in Loadout.
4. Loadout assigns that item to an equipment slot.
5. The selected configuration is saved as a profile.
6. The native DLL applies the configured equipment to CS2.

### Example

A player owns three AK-47 skins:

- AK-47 | Redline
- AK-47 | Neon Rider
- AK-47 | Vulcan

Armory displays all three skins.

Loadout lets the player select one of those skins for the AK-47 equipment slot.

Choosing Redline in Loadout does not remove Neon Rider or Vulcan from Armory. All three remain in the collection.

---

## 5. DOMINATE — Central Launcher and Entry Point

DOMINATE is the central launcher and startup orchestrator for N1SSAN.

The intended user journey is:

**Welcome → DOMINATE → Loadout or Armory**

There is no separate N1SSAN screen between DOMINATE and these destinations.

### Step 1 — Welcome

The user launches N1SSAN and arrives at the Welcome screen.

From there, the user proceeds into DOMINATE.

### Step 2 — DOMINATE System Checks

DOMINATE performs its three required system checks.

- All three checks must pass.
- The destination selection screen must not appear until all three checks have passed.
- If a check fails, DOMINATE must handle the failure and prevent entry into either destination.

### Step 3 — Destination Selection

After all three checks pass, DOMINATE presents two choices:

**LOADOUT**

- Opens the Loadout interface.
- Provides equipment configuration functionality.
- Uses shared inventory data managed by Armory.

**ARMORY**

- Opens the Armory interface.
- Provides inventory browsing and management functionality.
- Does not automatically change the player's equipped configuration.

These are two destinations within the same N1SSAN application session, not separate products.

---

## 6. Native DLL Injection and Runtime Initialization

The native DLL is responsible for both Loadout and Armory native functionality.

The intended integration flow is:

1. DOMINATE completes its required system checks.
2. The user selects Loadout or Armory.
3. DOMINATE ensures the N1SSAN native DLL is injected into CS2 through Injector v2.
4. The native DLL initializes its required components.
5. The DLL reports its runtime status.
6. The selected web interface becomes available and communicates through the N1SSAN Web Bridge.

The native DLL must not be independently injected once for Loadout and again for Armory.

**There should be one N1SSAN native runtime per CS2 process, shared by both destinations.**

Successful injection alone does not mean initialization is complete. DOMINATE should distinguish between injection success and verified native runtime readiness.

---

## 7. Profile Switching After Injection — Critical Runtime Requirement

This is a fundamental requirement of N1SSAN.

**Injecting the DLL must not lock the user into one saved profile.**

Even after the DLL has been injected and initialized, users must be able to switch between saved profiles and make profile changes while using either Loadout or Armory.

This is allowed as long as the user is not currently in an active CS2 match.

### Expected behavior

For example:

1. The user launches N1SSAN.
2. DOMINATE completes its checks.
3. The DLL is injected and reaches READY.
4. The user enters Loadout.
5. The user switches from Profile A to Profile B.
6. The DLL receives the profile change and updates the relevant runtime configuration.
7. The user can switch again to Profile C, Profile D, or any other saved profile.
8. The process continues without reinjecting the DLL.

The same principle applies when the user enters Armory.

### Runtime responsibilities

The native DLL must be designed to:

- Receive profile-switch requests from the application.
- Load and validate the selected saved profile.
- Update the active equipment configuration.
- Synchronize relevant configuration changes with the native runtime.
- Handle repeated profile switches during the same session.
- Avoid stale configuration or duplicated runtime state.
- Preserve inventory data and other saved profiles.
- Report success or failure to the application.

### Match-state restriction

Profile changes are permitted while the user is outside an active match.

When an active match is detected:

- Do not apply profile changes that are restricted during active gameplay.
- Do not corrupt or partially overwrite the active configuration.
- Communicate the restriction clearly to the UI.
- Preserve the requested saved profile and existing data.
- Allow the user to make the change once the restriction is lifted.

The exact match-state detection and safe update mechanism must be verified against the existing native implementation before development.

### Non-negotiable rule

**Profile switching is a runtime configuration operation, NOT a DLL injection operation.**

Do not reinject the DLL, restart CS2, or recreate the native runtime every time the user changes profiles.

---

## 8. Shared Data, Separate Responsibilities

Both systems use the same N1SSAN ecosystem, but their responsibilities must remain independent.

| Area                             | Loadout                        | Armory                      |
| -------------------------------- | ------------------------------ | --------------------------- |
| Main purpose                     | Equipment configuration        | Inventory management        |
| Displays                         | Equipment slots and selections | Complete item collection    |
| Selects equipped items           | Yes                            | No                          |
| Manages item collection          | No                             | Yes                         |
| Reads inventory data             | Yes                            | Owns/manages inventory data |
| Manages saved equipment profiles | Yes                            | No                          |
| Manages item metadata            | Consumes relevant data         | Yes                         |
| Changes active equipment         | Yes, through DLL               | No direct assignment        |
| Requires separate DLL            | No                             | No                          |

### Data integrity rules

1. Do not duplicate inventory data between the systems.
2. Loadout must reference items from shared inventory data.
3. Equipping or unequipping an item must update the Loadout configuration, not delete or modify the underlying inventory item.
4. Switching profiles must not overwrite other saved profiles.
5. Changes to one system must not unintentionally break the other.
6. Shared data models and native services should be reused where appropriate.

---

## 9. Repository and Component Responsibilities

| Component           | Responsibility                                                                    |
| ------------------- | --------------------------------------------------------------------------------- |
| `n1ssan-launcher`   | WPF launcher, Welcome, DOMINATE, checks, destination selection, process lifecycle |
| `n1ssan-injectorv2` | Native DLL injection and injection verification                                   |
| `CS2-n1ssan`        | Native DLL containing both Loadout and Armory logic                               |
| `n1ssan-web-bridge` | Web interfaces, bridge communication, profiles and application integration        |
| Loadout             | Equipment configuration interface                                                 |
| Armory              | Inventory management interface                                                    |

The old `n1ssan-injector` repository is superseded. Use `n1ssan-injectorv2` as the canonical injector project.

---

## 10. Complete User Journey

```text
                 WELCOME
                    |
                    v
                 DOMINATE
                    |
             SYSTEM CHECKS
                    |
             +------+------+
             |             |
          FAILED          PASSED
             |             |
          BLOCKED          v
                    DESTINATION
                     SELECTION
                         |
                   +-----+-----+
                   |           |
                   v           v
                LOADOUT      ARMORY
                   |           |
                   v           v
              Equipment    Inventory
              Management   Management
                   |           |
                   +-----+-----+
                         |
                         v
                 SHARED NATIVE DLL
                         |
                  CS2 RUNTIME READY
                         |
                         v
                 PROFILE SWITCHING
                 (Outside active match)
                         |
                 DLL updates runtime
                 without reinjection
```

---

## 11. Non-Negotiable Development Rules

1. Preserve the Welcome → DOMINATE → Loadout/Armory architecture.
2. Do not introduce a separate N1SSAN screen.
3. Do not merge Loadout and Armory into one interface.
4. Do not duplicate their business logic.
5. Keep both native systems inside the same N1SSAN DLL.
6. Use Injector v2 as the canonical injector.
7. Do not inject the DLL separately for each destination.
8. Do not require reinjection for profile switching.
9. Allow repeated profile changes outside active matches.
10. Prevent restricted profile changes during active matches.
11. Preserve existing saved profiles and inventory data.
12. Maintain clear runtime status reporting between the DLL, bridge, injector, and DOMINATE.
13. Inspect existing source code before modifying anything.
14. Do not claim a feature is implemented or working without verifying the actual code and test evidence.

## 12. Final Architectural Principle

**DOMINATE controls entry and application lifecycle.**

**Armory manages what the player owns.**

**Loadout manages what the player equips.**

**The native DLL powers both systems and handles runtime configuration changes.**

**Injector v2 initializes the DLL once per CS2 process.**

**Profile switching updates the existing runtime; it does not trigger another injection.**

These responsibilities must remain distinct and work together as one integrated N1SSAN application.
