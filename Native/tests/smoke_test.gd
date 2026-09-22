extends SceneTree

var checks: int = 0
var failures: int = 0
var game: Node3D

func _initialize() -> void:
	call_deferred("run")

func check(value: bool, description: String) -> void:
	checks += 1
	if not value:
		failures += 1
		push_error("FAIL: " + description)
	else:
		print("PASS: " + description)

func frames(count: int) -> void:
	for i in count:
		await physics_frame

func run() -> void:
	var scene = load("res://main.tscn")
	if not scene:
		quit(1)
		return
	game = scene.instantiate()
	root.add_child(game)
	await frames(3)
	game.save_store.base = "user://automated_smoke"
	for suffix in ["_a.json", "_b.json", "_a.json.tmp", "_b.json.tmp"]:
		DirAccess.remove_absolute(game.save_store.base + suffix)
	check(game.initialized, "scene initialized")
	check(get_nodes_in_group("citizens").size() == 28, "24 ambient citizens + 4 story citizens")
	check(get_nodes_in_group("vehicles").size() == 7, "player car + 6 traffic vehicles")
	check(game.markers.size() == 8, "eight interactive locations")
	check(game.quest_data.size() == 3, "three quest chains")
	game.new_game()
	await frames(15)
	check(game.player.is_on_floor(), "player supported by ground collision")
	var before = game.player.global_position
	Input.action_press("forward")
	await frames(35)
	Input.action_release("forward")
	await frames(5)
	check(game.player.global_position.distance_to(before) > 1.0, "walking input moves character")
	game.player.global_position = game.car.global_position + game.car.global_basis.x * 3.0 + Vector3.UP * 0.25
	game.player.velocity = Vector3.ZERO
	game.car.speed = 0
	await frames(3)
	game.enter_vehicle()
	check(game.in_vehicle and game.car.driven, "vehicle possession")
	before = game.car.global_position
	Input.action_press("forward")
	await frames(50)
	Input.action_release("forward")
	check(game.car.global_position.distance_to(before) > 1.0, "vehicle accelerates and travels")
	check(not game.exit_vehicle(), "cannot exit moving vehicle")
	game.car.speed = 0
	game.car.velocity = Vector3.ZERO
	check(game.exit_vehicle(), "safe exit from stopped vehicle")
	check(game.player.visible and not game.player.collision.disabled, "player restored after exit")
	game.player.weapon_visible = true
	var ammo = game.player.magazine
	check(game.player.try_fire(), "weapon fires a ballistic projectile")
	check(game.player.magazine == ammo - 1, "ammo deducted once")
	check(not game.player.try_fire(), "fire interval prevents immediate duplicate shot")
	game.player.reload()
	await frames(85)
	check(game.player.magazine == 15 and game.player.reserve == 119, "reload transfers reserve ammo")
	game.player.weapon_visible = false
	# Exercise every mission step through the actual interaction entry point.
	for i in 9:
		var target: Vector3 = game.markers[game.current_target()].position
		game.player.global_position = target + Vector3(0, 0.2, 2.6)
		game.player.velocity = Vector3.ZERO
		await frames(2)
		game.interact()
		check(game.mode == "dialogue", "mission interaction " + str(i + 1))
		game.close_dialogue()
	check(game.quest == 3 and game.step == 0, "all three quests complete")
	check(game.money == 1200, "mission rewards total exactly 1050")
	game.interact()
	game.close_dialogue()
	check(game.money == 1200, "completed quest cannot pay twice")
	game.money = 700
	check(game.save_game(false), "first save slot write")
	game.money = 900
	check(game.save_game(false), "second save slot write")
	game.money = 1
	check(game.load_game() and game.money == 900, "load restores latest valid snapshot")
	var a = game.save_store.read_slot(game.save_store.base + "_a.json")
	var b = game.save_store.read_slot(game.save_store.base + "_b.json")
	var latest_path = game.save_store.base + ("_a.json" if a.generation > b.generation else "_b.json")
	var file = FileAccess.open(latest_path, FileAccess.WRITE)
	file.store_string("{broken")
	file.close()
	check(game.load_game() and game.money == 700, "corrupt latest save falls back to older valid slot")
	game.weather = 2
	game.hour = 23
	game.rain_strength = 1
	game.update_environment()
	check(game.rain.emitting and game.sun.light_energy < 0.2, "night and rainy weather update environment")
	game.player.take_hit(120)
	check(game.player.health == 100 and game.money == 675, "death recovers player with stated fee")
	for suffix in ["_a.json", "_b.json", "_a.json.tmp", "_b.json.tmp"]:
		DirAccess.remove_absolute(game.save_store.base + suffix)
	print("OPENWORLD_SMOKE_RESULT checks=%d failures=%d" % [checks, failures])
	game.queue_free()
	await process_frame
	quit(1 if failures > 0 else 0)
