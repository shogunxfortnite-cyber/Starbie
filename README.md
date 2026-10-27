## schematic

<img width="1549" height="947" alt="Screenshot 2026-10-06 095604" src="https://github.com/user-attachments/assets/007b476b-68cb-442f-b659-598129f221c1" />

## PCB

<img width="1323" height="868" alt="Screenshot 2026-10-26 091755" src="https://github.com/user-attachments/assets/4aa196ce-4bea-4a5e-b40b-428cb4102142" />

## 3D view

<img width="1724" height="927" alt="3dd" src="https://github.com/user-attachments/assets/52da45d5-e76c-4210-8f15-6a25485de5d0" />


## What is it (my uniqueness)
A tiny motion-controlled digital pet that lives on a small screen and acts like Koro-sensei from Assassination Classroom. It wanders on its own, reacts to tilt and shake, and remembers how it feels after you unplug it.

One file. Open it in Arduino IDE, tweak a few numbers at the top, upload to a Seeed XIAO ESP32-C3. All settings sit in one block at the top so you never have to hunt through the code.

He has three feelings: mood, stamina, and sweets. They stay between 0 and 100 and never drift on their own. Nothing drains while he sits there. He only changes when you do something to him. He walks left and right forever and never leaves the screen.

Two buttons. One opens a menu, the other peeks at his stats. The stats button never changes anything about him.

Press the menu button and a circle appears with four options around it and a ball in the center. Tilt the board and the ball rolls toward the tilt; whatever it reaches lights up. Press again and he does that thing. The menu figures out which way you are holding the board the moment it opens, so it works sideways on the couch. No recalibration.

TEACH makes him stand still and lecture with a little chalkboard wobbling above his head. Lasts almost a minute. Tires him a bit and he enjoys it.

TICKLE makes him bolt back and forth twice at high speed with motion lines behind him and "Nyu?" / "Nufu" floating up. Most fun and most exhausting.

SNACK is his favorite. A happy hop and his sweets jump way up. Biggest gain in the menu.

PRAISE makes him blush and float "Nufufu." Costs no stamina, just makes him feel good.

Shaking him is not affection here, it is mockery. He gets grumpy, mood drops, and he stops to glare at you. Stamina goes up a little because his tentacles tense. There is a cooldown so one wiggle is not twenty insults.

His stats screen shows three bars with numbers, a small TARGET:1 line, and temperature and humidity at the bottom or dashes if the sensor is missing.

Missing parts are fine. No motion sensor and the menu just does not tilt. No temperature sensor and the stats show dashes. Only the screen is required.

The sprite is a small round face with a stripe across the top, two tiny eyes, a wide grin, and a hint of tentacles. At this size it reads as him if you know who he is, and as a friendly round creature if you do not.

Everything is timed, nothing is paused. He walks, lectures, hops, runs, and blushes while the buttons stay responsive and the screen keeps refreshing smoothly. Pressing a button mid-animation feels instant.

He saves constantly. Unplug him, leave him in a drawer for a week, plug him back in, and he is exactly as grumpy or as happy as you left him. One switch at the top of the file gives you a fresh start.

The settings at the top are the only part a beginner needs to touch, each with a short note saying what it is for. Change how fast he walks, how long a lecture lasts, how much a snack cheers him up, or which way the menu tilts by editing one line and re-uploading.

A small, stubborn, sweet-loving teacher who paces around a tiny screen, gets offended when you shake him, runs at absurd speed when you tickle him, and remembers how he felt the last time you saw him.



## BOM



| Item | Quantity | Price | Link |
| :--- | :---: | ---: | :--- |
| Xiao ESP32 C3 | 1 | $10.00 | https://robu.in/product/seeed-studio-xiao-esp32c3-tiny-mcu-board-with-wi-fi-and-ble-battery-charge-supported-power-efficiency-and-rich-interface |
| MPU6050 | 1 | $3.50 | https://amzn.in/d/0dZdvS9x |
| 0.96 OLED | 1 | $3.00 | https://robu.in/product/0-96-inch-i2c-iic-oled-lcd-module-4pin-with-vcc-gnd-blue |
| DHT 11 module | 1 | $0.7 | https://robu.in/product/dht11-digital-relative-humidity-temperature-sensor-module |
| MX Switches | 2 | $5.00 | https://amzn.in/d/0bCMinFX |
| Keycaps | 2 | $3.00 | https://amzn.in/d/0em6eUiV |
| 10k Resistor | 1 | $0.10 | https://robu.in/product/mf25-10k-multicomp-pro-through-hole-resistor-10-kohm-250-mw-%c2%b1-1-axial-leaded-250-v |
| Custom PCB | 1 | $6.30 | robu.in |
| Total | | $31.60 | |



## Note for reviewers 
seperate units of keycaps and mx switches are not found in my region so the total price,includes the bundle.sorry i couldn't find seperate units.

## How to code

We can either use the prebuilt libraries or use a custom code.

## controlled by
It is controlled by 2 switches and motion with the help of the mpu6050 module.

## Working
starbie is a tiny motion-controlled digital pet — basicly a desktop tamagotchi with minimal gameplay buttons. instead of pressing controls, you interact with it by physically moving the board around.

the brain of it is a seeed xiao esp32-c3, which runs the logic and pushes stuff to the oled. the mpu6050 reads motion — tilting, shaking, flipping the board, leaving it sitting still on your desk — all of that gets fed in and changes how the pet reacts on screen. so like if you tilt it one way the pet might get dizzy or happy, shake it and it gets startled, that sort of thing. makes it feel more alive then just mashing buttons.

theres also a dht sensor hooked up that reads temp and humidity, and those values also nudge the pet's state a bit (not a huge gameplay thing but it adds some flavor). the two switches handle the basic stuff — feeding, resetting, that kind of interaction.

whole thing was designed to be compact and portable-ish. pcb is small, case is 3d printed with a keychain hole so you can actually carry it around instead of it just sitting on your desk forever.

one thing to note — the motion part is a bit twitchy depending on how you set the mpu6050 thresholds. i had to tune mine a few times before the pet stoped reacting to every tiny vibration on the table lol. also the oled refresh can lag a little if you poll the sensors too fast, so the loop timing matters more then you'd think. and ngl the first version i built kept freezing every few minuts until i relised the wire library needed a delay in the setup, so watch out for that


## Note to the reviewers 
You can edit this code to make your own custom tamagochi.

## secrets 
This is made from a guide available at half life hack club. Pls guys if your under 18 and above 13 or 13 you can join this club.

## what did I use
I used kicad for making the PCB schematic and routing the PCB. For the code I used the prebuolt template and edited it a bit 

## libraries required 

Wire

Adafruit_GFX

Adafruit_SSD1306

Adafruit_MPU6050

DHT


## code

You guys can use either arduino ide or esp if for this.

math

## half life
This is built for half life🙃🫠🙃🫠🙃🙃 .
