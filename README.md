# CannonShooter
This project is a small, polished 2D arcade game, Cannon Shooter 2.0 built with SDL. The player controls cannon that can move left and right and aim, shooting enemies that spawn around the playfield. The game includes a friendly login and welcome flow, animated enemy sprites (or fallback shapes), soundless but smooth visuals, and clear on-screen feedback: health bars, score, level text, and victory/defeat screens that show earned stars. Controls are simple and intuitive, and enemies have basic AI that times their shots and aims at the player. Performance minded details such as a pooled bullet system (recycling bullet objects instead of repeatedly allocating) and texture initialization with error checks make the game responsive and robust. On the systems side, player profiles and scores persist in a compact CSVlike storage and a leaderboard module that sorts, highlights the current player, and gracefully handles missing or malformed files. The codebase is neatly modular: separate classes handle bullets, enemies, player, login, game loop, and storage, which makes it easy to read, maintain, and extend (for example adding new enemy types, weapons, or sound). Overall, the project balances approachable gameplay with solid engineering choices which are ideal as a learning project or as a foundation to add polish like sound effects, power-ups, or online leaderboards.
## INTRODUCTION:
In Cannon Shooter, you play as a cannon operator trying to survive against
enemies coming from different sides. You move, aim, and shoot to protect
yourself and clear each level. As you go forward, enemies get faster and
harder to beat. You earn points by hitting them and staying alive. Your
aim, timing, and quick moves decide how far you can go in the game.
## GAMEPLAY MECHANICS:
1. Login & Profile Initialization
The game begins with a login screen rendered by
Login::updateAndRender(), where the player enters a name.
On “START”, main() loads existing profiles through
StorageHash::loadFromFile("profiles.dat"), initializes or creates a Profile
with score 0 if new, and displays a welcome screen showing current
score/rank from StorageHash::getScore() and getRank().
2. Welcome Screen & Score Carryover
After login, a 2-second welcome screen (in main.cpp) shows the player’s
persistent score and leaderboard rank using profile data. Crucially,
game.setPermanentScore(savedScore) stores the prior score Player::score,
meaning each session starts fresh. Profile data is saved before gameplay
begins through StorageHash::saveToFile().
 3. Core Gameplay Loop
The Game class drives gameplay:
Game::handleEvents() processes movement (←/→), aim-and-shoot
through S key (triggers shootDirection() toward nearest alive enemy), and
mouse input for UI.
Game::update() runs physics, updates BulletManager::update() (checking
collisions with Enemy::isAlive()), awards +10 per kill through
player.addScore(), and triggers enemy respawns at milestones (e.g.,
spawnEnemies(2) after 1st/2nd kill).
Health-based damage: player.takeDamage() on enemy bullet hit (tracked
through static BulletManager::playerHitThisFrame).
4. Enemy & Bullet System (Custom Pool-Based Manager)
Enemies (Enemy) are stored in a std::vector<Enemy>, each with health,
alive state, and auto-firing logic (Enemy::shouldShoot() → shoot()). Bullets
are managed through a custom fixed-size pool (BulletManager) using a
circular free queue no dynamic allocation. Collision uses
SDL_HasIntersection(); on hit, Enemy::hit() sets alive = false and justKilled
= true, which Game::update() detects to increment totalDeaths and score.
5. Win/Loss Conditions & Scoring
Victory occurs at 10 total kills (totalDeaths >= 10); defeat at 0 player
health. Post-game, Game::render() shows stars based on kill ratio (e.g., 9+
kills = 3 stars), and saves the session score through
StorageHash::insertOrUpdate() and saveToFile(). The “LEADERBOARD”
button loads rankings through Leaderboard::loadFromFile(), which
internally sorts profiles by score/name and renders top entries with
current player highlighted.
6. Restart & Persistence Design
Pressing SPACE on end screens resets: enemies.clear(), player = Player(...),
and spawnEnemies(3). Profile data is persisted across runs in profiles.dat
(CSV: name,score). The leaderboard is reloaded on demand through
leaderboard.loadFromFile(), ensuring live rank updates.
7. Player Mechanics & State Management
The Player class (initialized in Game::init() through Player(400, 610, 90, 90)
and initTexture()) handles movement with handleInput() (←/→ keys),
boundary clamping in update(), and aim-assist targeting when the S key
is pressed which calculates the nearest alive enemy through distancesquared comparison and orients the cannon using setAngle() (converting
vector to degrees). On hit, player.takeDamage() reduces health (max 3)
and deducts 5 points (floored at 0); the health bar renders dynamically in
Player::render(), changing color (green → yellow → red) as health drops.
## ACHIEVEMENT AND REWARD SYSTEM:
The achievement and reward system tell rewards accurate play, survival,
and clearing levels. Players get points when enemies die, lose a little score
when they take damage, and earn a star rating at the end of a run. Final
results (score and stars) are saved to the local storage and shown on the
leaderboard so players can track progress over time.
### Core Mechanics:
● Points per kill: Each enemy death gives 10 points through
player.addScore() (called in Game::update())
● Penalty on hit: Taking damage reduces health and subtracts 5 points
using Player::takeDamage() function.
● End-of-run stars: Star count is computed in Game::render() using
totalDeaths / 10.0f.
● Save & leaderboard: Final score is stored and updated with
storage.insertOrUpdate() and written to disk with
storage.saveToFile("profiles.dat"); the leaderboard shows saved results.
## GAME MAP:
The game map is a simple 2D battlefield designed for clear and fast
gameplay. The player stands on a flat ground area, while enemies appear
from fixed points around the arena. Background images are used in menus
and screens, but the fighting area stays plain to keep focus on action.
### Key Areas:
1. Player ground platform
2. Top enemy spawn point
3. Left enemy spawn point
4. Right enemy spawn point
5. Bottom support/spawn zone 
