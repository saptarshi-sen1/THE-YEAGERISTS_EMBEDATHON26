# 🦐 ShrimpHub: The Encrypted Reef Challenge

## A Hardware–Embedded Systems Hackathon

---

## 🌊 Narrative Theme: **The Shrimp Riddle**

### 🧬 The Legend of ShrimpHub Reef

Deep within the digital ocean lies the **ShrimpHub Reef**—an encrypted sanctuary where bioluminescent shrimp communicate through intricate protocols of light pulses and timed signals. For generations, the colony has guarded a secret: a **hidden video message**, encoded within perfectly synchronized light patterns.

Your research station has intercepted a cryptic transmission:

> *“The reef is calling. The shrimp have a message. Will you answer?”*

To prove your understanding of the reef’s language, the colony challenges you with **six trials**. Each task unlocks a deeper layer of the reef’s secrets. Only by completing all six—**in sequence**—can you retrieve the final video and decode the ancient message.

> **The Riddle**
> *“Signals blink in the dark, pixels dance in sequence, windows open and close—can you hear the reef’s song?”*

---

## 🧭 The Journey Ahead

Your mission unfolds through **six escalating challenges**, each building upon the previous:

1. **🎵 The Timing Keeper** – Master the reef’s heartbeat (rhythmic LED patterns)
2. **⚔️ The Priority Guardian** – Respond instantly to distress while maintaining background harmony
3. **🪟 The Window Synchronizer** – Act within the reef’s fleeting open window
4. **🔐 The Steganography Decoder** – Reveal messages hidden within images
5. **🎨 The Pixel Sculptor** – Reconstruct visuals using optimal transport
6. **📺 The Sequence Renderer** – Display the reef’s final video, frame by frame

⚠️ **Warning:** Tasks must be completed **strictly in order**. Skipping or reordering stages will invalidate progress.

---

## 🆔 Team Identification & Rules

### Team Identifiers

* **TeamID**: `<leader_first_name><team_name>` (lowercase, no spaces)
  *Example:* Leader **Gajendra**, Team **Oceans11** → `gajendraoceans11`

* **ReefID**: `<leader_email_prefix><team_name>` (lowercase, no spaces)
  *Example:* Email **[sherk@reef.com](mailto:sherk@reef.com)**, Team **Oceans11** → `sherkoceans11`

---

### ⚠️ Safety & Integrity Rules (Strict)

* **Any burning or mishandling of components results in immediate disqualification.**
* **No spare components will be provided.**
* **Using external components without prior jury approval leads to disqualification.**

---

### 📁 Documentation & Submission Standards

* Maintain **clear documentation** for each task: code, logs, images, and explanations.
* Include justification and validation for every step.
* All material must be uploaded to **GitHub**.

**Repository Naming Convention:**

```
TEAMNAME_EMBEDDATHON26
```

**Required Collaborator:**

```
ash29062
```

Submissions that are ambiguous, incomplete, or poorly justified may receive penalties or disqualification.

---

## 🧩 Task 1: The Timing Keeper 🎵

> *“Listen to the heartbeat of the reef.”*

### Objective

Replicate the reef’s rhythmic bioluminescent signals using an RGB LED. Each color channel represents an independent timing sequence that must remain perfectly synchronized over time.

### Input Signal

**MQTT Topic:**

```
shrimphub/led/timing/set
```

**Message Format:**

```json
{
  "red":   [500, 200, 300],
  "green": [400, 300, 200],
  "blue":  [600, 100, 400]
}
```

### Requirements

* Implement precise **ON–OFF LED cycles** for each RGB channel.
* Use **FreeRTOS timing primitives** (e.g., `vTaskDelayUntil()` preferred).
* System must run continuously for **5 minutes with no drift**.

### Evaluation Criteria

* Timing accuracy within **±5 ms**
* No cumulative drift over time
* Video evidence with a **running stopwatch visible**

### Success Condition

Correct timing triggers a **broker confirmation**, unlocking Task 2.

---

## 🧩 Task 2: The Priority Guardian ⚔️

> *“Two songs, one answer—respond to the cry in the dark.”*

### Objective

Demonstrate **real-time preemption** by handling urgent distress signals while continuously processing background data.

---

### Low-Priority Task: Background Chorus

* **Topic:** `krillparadise/data/stream`
* **Payload:** Floating-point values
* **Task:** Maintain a rolling average of the **last 10 values**
* **Output:** Print updated average to serial

**Example:**

```
Input: 23.5  → Avg: 23.5
Input: 24.1  → Avg: 23.8
Input: 22.8  → Avg: 23.47
```

---

### High-Priority Task: Distress Signal

* **Topic:** `<TeamID>`
* **Payload:** `"CHALLENGE"`
* **Arrival:** Random (2–10 s intervals)
* **Response Deadline:** ≤ **250 ms**

**Acknowledgment Publish:**

* **Topic:** `<ReefID>`

```json
{
  "status": "ACK",
  "timestamp_ms": <current_millis>
}
```

---

### Priority Architecture

| Priority | Task              |
| -------- | ----------------- |
| 3 (High) | Distress Handler  |
| 2        | MQTT Dispatcher   |
| 1 (Low)  | Background Chorus |

### Success Condition

* **10 valid distress acknowledgments**
* Broker publishes the **window challenge code** for Task 3

**Evidence:** Serial logs, LED indicators, stopwatch-visible video.

---

## 🧩 Task 3: The Window Synchronizer 🪟

> *“Catch the moment between moments.”*

### Objective

Synchronize a **physical button press** with a short-lived digital window signal.

### Signal Details

* **Topic:** `<window_code>`
* **Message:** `{ "status": "open" }`
* **Duration:** 500–1000 ms

### Hardware Requirements

* GPIO button input
* Software debounce ≈ **20 ms**

### Response

* **Topic:** `cagedmonkey/listener`

```json
{
  "status": "synced",
  "timestamp_ms": <sync_time>
}
```

### Tolerance & Validation

* Timing tolerance: **±50 ms**
* Recommended: FreeRTOS **Event Groups**

### Success Condition

* **3 successful synchronizations**
* Broker publishes steganography challenge code

---

## 🧩 Task 4: The Silent Image 🔐

> *“Not everything worth knowing announces itself.”*

### Objective

Reconstruct a transmitted image and uncover a **hidden message** embedded within it.

### Phase 1: Signal the Reef

* **Topic:** `kelpsaute/steganography`

```json
{
  "request": "<message_from_task_3>",
  "agent_id": "<TeamID>"
}
```

### Phase 2: Data Reconstruction

* Reassemble received fragments
* Validate image integrity and format

### Phase 3: Hidden Pattern Analysis

* Analyze **relative color relationships**, not absolute values
* Extract concealed information (e.g., LSB-based steganography)

### Phase 4: Discovery

* Decode recovered text
* Obtain `target_image_url` for Task 5

---

## 🧩 Task 5: The Pixel Sculptor 🎨

> *“Rearrange the reef’s colors to match its ancient blueprint.”*

### Objective

Transform a source image to match a target image using **Optimal Transport (OT)**.

### Workflow

1. Fetch target image (non-blocking HTTP)
2. Load source image from `coralcrib/img`
3. Compute OT transport plan
4. Apply transformation
5. Validate using **SSIM ≥ 0.70**
6. Publish transformed image

### Publish Topic

```
<TeamID>_<ReefID>
```

### Evidence Required

* Transformation logs
* SSIM score output
* MQTT publish confirmation

---

## 🧩 Task 6: The Sequence Renderer 📺

> *“Display the reef’s final song—frame by frame.”*

### Objective

Render the reef’s final video on an **OLED display**, synchronized in real time.

### Process

1. Subscribe to `<sequencer_code>`
2. Receive frame packets:

   * `frame_no`
   * `data`
   * `delay_ms`
3. Display frames with **±20 ms accuracy**
4. Publish acknowledgment for each frame

### Success Condition

* Video loops **3+ times** without drift
* Broker publishes final congratulatory message

---

## 🎉 Completion & Final Revelation

Upon successful completion of all six tasks:

* The broker releases the **decoded secret of ShrimpHub Reef**
* Your team is recognized as a **Trusted Ally of the Shrimp Colony** 🦐

---

## 🏆 Judging & Scoring

### Functional Scoring (200 Points)

| Task | Description                  | Points |
| ---- | ---------------------------- | ------ |
| 1    | Timing accuracy & stability  | 10     |
| 2    | Priority handling & response | 10     |
| 3    | Window synchronization       | 20     |
| 4    | Steganography decoding       | 40     |
| 5    | Optimal transport & SSIM     | 60     |
| 6    | OLED sequence rendering      | 60     |

### Additional Evaluation

* **Code Quality:** 70 points (architecture, robustness, documentation)
* **Performance & Innovation:** 15 points
* **Presentation & Video Evidence:** 15 points

---
