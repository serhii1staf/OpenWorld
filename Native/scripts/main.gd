extends Node3D

const Player = preload("res://scripts/player.gd")
const Car = preload("res://scripts/vehicle.gd")
const Citizen = preload("res://scripts/citizen.gd")
const WorldBuilder = preload("res://scripts/world_builder.gd")
const Visuals = preload("res://scripts/visuals.gd")
const Store = preload("res://scripts/save_store.gd")
const HUD = preload("res://scripts/hud.gd")

var mode: String = "title"
var player: CharacterBody3D
var car: CharacterBody3D
var in_vehicle: bool = false
var camera: Camera3D
var camera_pivot: Node3D
var spring: SpringArm3D
var camera_yaw: float = -0.25
var camera_pitch: float = -0.23
var camera_shake: float = 0.0
var camera_far: bool = false
var hud: Control
var builder: RefCounted
var environment: Environment
var sun: DirectionalLight3D
var rain: CPUParticles3D
var hour: float = 16.2
var rain_strength: float = 0.0
var weather: int = 0
var weather_timer: float = 0.0
var elapsed: float = 0.0
var environment_timer: float = 0.0
var money: int = 150
var quest: int = 0
var step: int = 0
var quest_data: Array = []
var markers: Dictionary = {}
var nearest_id: String = ""
var interaction_timer: float = 0.0
var beacon: Node3D
var toast: String = ""
var toast_timer: float = 0.0
var dialogue_title: String = ""
var dialogue_text: String = ""
var show_map: bool = false
var show_stats: bool = false
var inventory: Array = []
var save_store = Store.new()
var quality: int = 1
var volume: float = 0.65
var fullscreen: bool = false
var music: AudioStreamPlayer
var ambient: AudioStreamPlayer
var sounds: Dictionary = {}
var bullets: Array = []
var bullet_nodes: Array = []
var initialized: bool = false
var session_started: bool = false
var danger: float = 0.0

func _ready() -> void:
	setup_input()
	setup_quests()
	setup_environment()
	builder = WorldBuilder.new()
	builder.build(self)
	player = Player.new()
	player.name = "Courier"
	player.game = self
	add_child(player)
	player.position = Vector3(0, 0.02, 19)
	car = Car.new()
	car.name = "CourierCar"
	car.game = self
	add_child(car)
	car.position = Vector3(12, -0.05, 3)
	car.rotation.y = -PI * 0.5
	setup_camera()
	setup_interactions()
	setup_population()
	setup_audio()
	setup_projectiles()
	var layer = CanvasLayer.new()
	add_child(layer)
	hud = HUD.new()
	hud.game = self
	layer.add_child(hud)
	load_settings()
	apply_quality()
	update_environment()
	initialized = true
	Input.mouse_mode = Input.MOUSE_MODE_VISIBLE

func setup_input() -> void:
	var bindings = {"forward": KEY_W, "back": KEY_S, "left": KEY_A, "right": KEY_D, "jump": KEY_SPACE, "sprint": KEY_SHIFT, "crouch": KEY_CTRL, "interact": KEY_E, "reload": KEY_R, "weapon": KEY_Q, "pause": KEY_ESCAPE, "map": KEY_M, "save": KEY_F5, "load": KEY_F9, "weather": KEY_F6, "time": KEY_T, "camera": KEY_V, "stats": KEY_F3}
	for action in bindings:
		if not InputMap.has_action(action):
			InputMap.add_action(action)
		var event = InputEventKey.new()
		event.physical_keycode = bindings[action]
		InputMap.action_add_event(action, event)
	if not InputMap.has_action("fire"):
		InputMap.add_action("fire")
	var mouse = InputEventMouseButton.new()
	mouse.button_index = MOUSE_BUTTON_LEFT
	InputMap.action_add_event("fire", mouse)

func setup_quests() -> void:
	quest_data = [
		{"name": "01 / Первая доставка", "reward": 200, "steps": [
			{"id": "dispatcher", "task": "Поговорите с диспетчером на площади", "line": "Добро пожаловать в Меридиан. Машина у дороги — твоя. Забери медицинский ящик со склада и доставь в клинику. Не спеши на перекрёстках."},
			{"id": "parcel", "task": "Заберите медицинский ящик в порту", "line": "Ящик опломбирован. Клиника находится к северо-западу от площади. Груз добавлен в сумку.", "item": "Медицинский ящик"},
			{"id": "clinic", "task": "Доставьте ящик в клинику", "line": "Всё на месте. Спасибо — сегодня этот груз очень нужен. Механик на востоке тоже ищет курьера."}]},
		{"name": "02 / Северная мастерская", "reward": 350, "steps": [
			{"id": "mechanic", "task": "Найдите механика Northline Garage", "line": "Для ремонта нужен блок питания. Его оставили между портовыми контейнерами. Привези его — заодно приведу твою машину в порядок."},
			{"id": "parts", "task": "Заберите блок питания на грузовом дворе", "line": "Номер на коробке совпадает с заказом. Осталось вернуться в мастерскую.", "item": "Блок питания"},
			{"id": "mechanic", "task": "Верните деталь механику", "line": "Отличная работа. Кузов и двигатель твоей машины отремонтированы. Егерь на севере просил помочь с радио на смотровой площадке."}]},
		{"name": "03 / Сигнал с хребта", "reward": 500, "steps": [
			{"id": "ranger", "task": "Посетите станцию егерей на севере", "line": "Погода меняется, а на хребте пропала связь. Забери аккумулятор на заправке и отвези к ретранслятору у смотровой площадки."},
			{"id": "battery", "task": "Заберите аккумулятор на заправке", "line": "Аккумулятор заряжен. От заправки поезжай на север, затем поверни к деревянной площадке.", "item": "Аккумулятор"},
			{"id": "lookout", "task": "Восстановите связь на смотровой площадке", "line": "Ретранслятор снова работает. Порт, клиника и лесная станция на связи. Все три заказа завершены. Теперь можно исследовать Меридиан без спешки."}]}]

func setup_environment() -> void:
	var world_env = WorldEnvironment.new()
	environment = Environment.new()
	environment.background_mode = Environment.BG_SKY
	var sky = Sky.new()
	var sky_material = ProceduralSkyMaterial.new()
	sky_material.sky_top_color = Color("709dab")
	sky_material.sky_horizon_color = Color("e3cba2")
	sky_material.ground_bottom_color = Color("58736f")
	sky_material.ground_horizon_color = Color("bbbd9c")
	sky.sky_material = sky_material
	environment.sky = sky
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color("b5c7c4")
	environment.ambient_light_energy = 0.7
	environment.tonemap_mode = Environment.TONE_MAPPER_FILMIC
	environment.fog_enabled = true
	environment.fog_density = 0.0018
	environment.fog_light_color = Color("a4b8b2")
	world_env.environment = environment
	add_child(world_env)
	sun = DirectionalLight3D.new()
	sun.name = "Sun"
	sun.light_color = Color("ffe0ad")
	sun.light_energy = 1.7
	sun.shadow_enabled = true
	sun.directional_shadow_max_distance = 100.0
	add_child(sun)
	rain = CPUParticles3D.new()
	rain.amount = 360
	rain.lifetime = 0.8
	rain.emission_shape = CPUParticles3D.EMISSION_SHAPE_BOX
	rain.emission_box_extents = Vector3(9, 2, 9)
	rain.direction = Vector3(0.15, -1, 0)
	rain.spread = 4
	rain.initial_velocity_min = 15
	rain.initial_velocity_max = 20
	rain.gravity = Vector3(0, -10, 0)
	var rain_mesh = BoxMesh.new()
	rain_mesh.size = Vector3(0.013, 0.55, 0.013)
	rain.mesh = rain_mesh
	rain.material_override = Visuals.mat("rain", Color("b4d3dc"), 0, 0.3)
	rain.emitting = false
	add_child(rain)

func setup_camera() -> void:
	camera_pivot = Node3D.new()
	add_child(camera_pivot)
	spring = SpringArm3D.new()
	spring.spring_length = 5.0
	spring.margin = 0.25
	camera_pivot.add_child(spring)
	spring.add_excluded_object(player.get_rid())
	spring.add_excluded_object(car.get_rid())
	camera = Camera3D.new()
	camera.fov = 67
	camera.far = 1100
	spring.add_child(camera)
	camera.current = true
	camera_pivot.position = player.position + Vector3(0, 1.6, 0)

func setup_interactions() -> void:
	add_marker("dispatcher", "Диспетчер", Vector3(-18, 0, 20), true)
	add_marker("parcel", "Медицинский ящик", Vector3(-80, 0, 112), false)
	add_marker("clinic", "Клиника", Vector3(-120, 0, -38), true)
	add_marker("mechanic", "Механик", Vector3(127, 0, 49), true)
	add_marker("parts", "Запчасти", Vector3(20, 0, 113), false)
	add_marker("ranger", "Егерь", Vector3(-136, 0, -163), true)
	add_marker("battery", "Аккумулятор", Vector3(124, 0, -117), false)
	add_marker("lookout", "Ретранслятор", Vector3(135, 0, -196), false)
	beacon = Node3D.new()
	add_child(beacon)
	var ring_mesh = TorusMesh.new()
	ring_mesh.inner_radius = 0.7
	ring_mesh.outer_radius = 0.8
	ring_mesh.rings = 24
	ring_mesh.ring_segments = 8
	var ring = MeshInstance3D.new()
	ring.mesh = ring_mesh
	ring.material_override = Visuals.mat("objective", Color("ffcd70"), 0, 1.2)
	beacon.add_child(ring)
	Visuals.cylinder(beacon, 0.045, 3.0, Vector3(0, 2.3, 0), Visuals.mat("objective", Color("ffcd70"), 0, 1.2))

func add_marker(id: String, title: String, pos: Vector3, person: bool) -> void:
	markers[id] = {"title": title, "position": pos}
	if person:
		var npc = Citizen.new()
		npc.game = self
		npc.story_actor = true
		npc.appearance = markers.size()
		npc.home = pos
		npc.destination = pos
		add_child(npc)
		npc.position = pos
		npc.rotation.y = PI
	else:
		Visuals.box(self, Vector3(0.85, 0.8, 0.65), pos + Vector3(0, 0.4, 0), Visuals.mat("parcel", Color("b29164")), true)
		Visuals.box(self, Vector3(0.87, 0.08, 0.1), pos + Vector3(0, 0.73, 0.34), Visuals.mat("label", Color("e1d9b5")))
		if id == "lookout":
			Visuals.cylinder(self, 0.07, 7, pos + Vector3(0.65, 3.5, 0), Visuals.mat("steel", Color("a1b2b4"), 0.65))
			Visuals.sphere(self, 0.3, pos + Vector3(0.65, 7, 0), Visuals.mat("antenna", Color("d4c4a2")))

func setup_population() -> void:
	var paths = [
		[Vector3(-76, 0, 8.5), Vector3(78, 0, 8.5)],
		[Vector3(-76, 0, 90), Vector3(78, 0, 90)],
		[Vector3(-108.5, 0, -75), Vector3(-108.5, 0, 76)],
		[Vector3(91.5, 0, -73), Vector3(91.5, 0, 82)],
		[Vector3(-78, 0, -91), Vector3(77, 0, -91)],
		[Vector3(118, 0, 160), Vector3(201, 0, 160)]]
	for i in 24:
		var npc = Citizen.new()
		npc.game = self
		npc.appearance = i
		var path = paths[i % paths.size()]
		npc.home = path[0]
		npc.destination = path[1]
		add_child(npc)
		npc.position = path[0].lerp(path[1], float(i / 6) / 4.0)
	var route_a = [Vector3(-103, 0, -3), Vector3(-103, 0, 103), Vector3(103, 0, 103), Vector3(103, 0, -3)]
	var route_b = [Vector3(-97, 0, 3), Vector3(97, 0, 3), Vector3(97, 0, 97), Vector3(-97, 0, 97)]
	for i in 6:
		var traffic = Car.new()
		traffic.game = self
		traffic.ai = true
		traffic.color = [Color("c1b78d"), Color("b96d51"), Color("5a747d")][i % 3]
		traffic.route = route_a if i % 2 == 0 else route_b
		var edge = int(i / 2)
		traffic.route_index = (edge + 1) % 4
		add_child(traffic)
		traffic.position = traffic.route[edge].lerp(traffic.route[traffic.route_index], 0.35) + Vector3(0, -0.05, 0)
		traffic.look_at(traffic.route[traffic.route_index] + Vector3(0, 0.3, 0))

func setup_audio() -> void:
	for id in ["shot", "step", "reload", "impact", "success"]:
		sounds[id] = load("res://assets/" + id + ".wav")
	music = AudioStreamPlayer.new()
	music.stream = load("res://assets/music.wav")
	music.volume_db = -21
	add_child(music)
	ambient = AudioStreamPlayer.new()
	ambient.stream = load("res://assets/ambient.wav")
	ambient.volume_db = -23
	add_child(ambient)

func setup_projectiles() -> void:
	var material = Visuals.mat("bullet", Color("ffe7a3"), 0, 2)
	for i in 48:
		var mesh = Visuals.sphere(self, 0.035, Vector3.ZERO, material)
		mesh.visible = false
		bullet_nodes.append(mesh)

func sound_effect(id: String, pos: Vector3, gain: float = 1.0) -> void:
	if DisplayServer.get_name() == "headless" or not sounds.has(id):
		return
	var sound = AudioStreamPlayer3D.new()
	sound.stream = sounds[id]
	sound.max_distance = 75
	sound.volume_db = linear_to_db(maxf(gain, 0.01)) - 8
	add_child(sound)
	sound.global_position = pos
	sound.finished.connect(sound.queue_free)
	sound.play()

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and mode == "play" and not show_map:
		camera_yaw -= event.relative.x * 0.0024
		camera_pitch = clampf(camera_pitch - event.relative.y * 0.0022, -1.05, 0.25)
	if event.is_action_pressed("pause"):
		if mode == "dialogue":
			close_dialogue()
		elif mode == "play":
			set_mode("pause")
		elif mode in ["pause", "settings"]:
			set_mode("play" if session_started else "title")
	if event.is_action_pressed("interact"):
		if mode == "dialogue":
			close_dialogue()
		elif mode == "play":
			interact()
	if mode != "play":
		return
	if event.is_action_pressed("reload"):
		player.reload()
	if event.is_action_pressed("weapon") and not in_vehicle:
		player.weapon_visible = not player.weapon_visible
		player.gun.visible = player.weapon_visible
	if event.is_action_pressed("map"):
		show_map = not show_map
		if show_map:
			set_mode("map")
	if event.is_action_pressed("save"):
		save_game()
	if event.is_action_pressed("load"):
		load_game()
	if event.is_action_pressed("weather"):
		weather = (weather + 1) % 3
		notify(["Ясная погода", "Морской туман", "Дождь над портом"][weather])
	if event.is_action_pressed("time"):
		hour = fmod(hour + 3.0, 24.0)
	if event.is_action_pressed("camera"):
		camera_far = not camera_far
	if event.is_action_pressed("stats"):
		show_stats = not show_stats

func _input(event: InputEvent) -> void:
	if mode == "map" and (event.is_action_pressed("map") or event.is_action_pressed("pause")):
		show_map = false
		set_mode("play")
		get_viewport().set_input_as_handled()

func _process(delta: float) -> void:
	if not initialized:
		return
	toast_timer = maxf(0.0, toast_timer - delta)
	camera_shake = maxf(0.0, camera_shake - delta * 0.6)
	var target = focus_position() + Vector3(0, 1.65 if not in_vehicle else 1.9, 0)
	camera_pivot.position = camera_pivot.position.lerp(target, minf(delta * 11, 1))
	if in_vehicle and mode == "play" and absf(car.speed) > 2 and Input.is_action_pressed("forward"):
		camera_yaw = lerp_angle(camera_yaw, car.rotation.y, delta * 0.65)
	camera_pivot.rotation = Vector3(camera_pitch, camera_yaw, 0)
	spring.spring_length = lerpf(spring.spring_length, (9.0 if camera_far else 6.7) if in_vehicle else (6.2 if camera_far else 4.6), minf(delta * 4, 1))
	camera.h_offset = sin(elapsed * 97.0) * camera_shake
	camera.fov = lerpf(camera.fov, 67.0 + absf(car.speed) * 0.25 if in_vehicle else 67.0, delta * 2.0)
	if mode == "play":
		elapsed += delta
		hour = fmod(hour + delta * 0.018, 24)
		weather_timer += delta
		if weather_timer > 90:
			weather_timer = 0
			weather = (weather + 1) % 3
		rain_strength = move_toward(rain_strength, 1.0 if weather == 2 else 0.0, delta * 0.12)
		danger = maxf(danger - delta, 0)
		environment_timer -= delta
		if environment_timer <= 0:
			environment_timer = 0.2
			update_environment()
		interaction_timer -= delta
		if interaction_timer <= 0:
			interaction_timer = 0.12
			update_nearest()
		if in_vehicle:
			player.global_position = car.global_position
	if quest < quest_data.size():
		beacon.visible = true
		beacon.position = markers[current_target()].position + Vector3(0, 0.15 + sin(elapsed * 2) * 0.06, 0)
	else:
		beacon.visible = false
	rain.global_position = focus_position() + Vector3.UP * 7
	hud.queue_redraw()

func _physics_process(delta: float) -> void:
	if mode != "play":
		return
	for i in range(bullets.size() - 1, -1, -1):
		var b = bullets[i]
		b.age += delta
		b.velocity.y -= 9.8 * delta
		var next: Vector3 = b.position + b.velocity * delta
		var query = PhysicsRayQueryParameters3D.create(b.position, next, 1, [player.get_rid()])
		var hit = get_world_3d().direct_space_state.intersect_ray(query)
		if not hit.is_empty() or b.age > 2.0:
			if not hit.is_empty() and hit.collider.has_method("take_hit"):
				hit.collider.take_hit(22.0, player.global_position)
			bullet_nodes[b.slot].visible = false
			bullets.remove_at(i)
		else:
			b.position = next
			bullet_nodes[b.slot].global_position = next

func update_environment() -> void:
	var day = clampf(sin((hour - 6.0) / 24.0 * TAU) * 3.0, 0.0, 1.0)
	sun.rotation_degrees = Vector3(-(hour - 6.0) * 15.0, -35, 0)
	sun.light_energy = (0.06 + day * 0.9) * (1.0 - rain_strength * 0.55)
	sun.light_color = Color("ffe0ad") if hour > 15 and hour < 19 else Color("d4e5df")
	environment.ambient_light_energy = 0.24 + day * 0.20
	environment.fog_density = lerpf(environment.fog_density, 0.011 if weather == 1 else 0.004 if weather == 2 else 0.0016, 0.08)
	var sky_material = environment.sky.sky_material
	sky_material.sky_top_color = Color("112536").lerp(Color("709dab"), day * (1.0 - rain_strength * 0.45))
	sky_material.sky_horizon_color = Color("43515c").lerp(Color("e3cba2"), day)
	rain.emitting = rain_strength > 0.1
	if builder:
		builder.window_mat.emission_energy_multiplier = 0.15 + (1 - day) * 1.2
		builder.dark.roughness = 0.85 - rain_strength * 0.42

func focus_position() -> Vector3:
	return car.global_position if in_vehicle else player.global_position

func current_target() -> String:
	return quest_data[quest].steps[step].id if quest < quest_data.size() else ""

func update_nearest() -> void:
	nearest_id = ""
	if in_vehicle:
		return
	var nearest = 3.6
	for id in markers:
		var distance = player.global_position.distance_to(markers[id].position)
		if distance < nearest:
			nearest = distance
			nearest_id = id
	if player.global_position.distance_to(car.global_position) < minf(4.0, nearest + 0.5):
		nearest_id = "vehicle"

func interact() -> void:
	if in_vehicle:
		exit_vehicle()
		return
	update_nearest()
	if nearest_id == "vehicle":
		enter_vehicle()
		return
	if nearest_id.is_empty():
		return
	dialogue_title = markers[nearest_id].title
	if nearest_id == current_target():
		var data = quest_data[quest].steps[step]
		dialogue_text = data.line
		if data.has("item"):
			inventory.append(data.item)
		step += 1
		if step >= quest_data[quest].steps.size():
			money += quest_data[quest].reward
			notify("Заказ завершён  /  +$" + str(quest_data[quest].reward))
			sound_effect("success", player.global_position, 1.0)
			if quest == 1:
				car.health = 100
			quest += 1
			step = 0
			inventory.clear()
		save_game(false)
	else:
		dialogue_text = "Здесь пока нет нового заказа. Текущая цель отмечена на карте золотым маркером."
	set_mode("dialogue")

func close_dialogue() -> void:
	set_mode("play")

func enter_vehicle() -> void:
	if player.global_position.distance_to(car.global_position) > 4.5 or absf(car.speed) > 1.0:
		return
	in_vehicle = true
	car.driven = true
	player.visible = false
	player.collision.disabled = true
	player.velocity = Vector3.ZERO
	player.reload_clock = 0
	player.weapon_visible = false
	player.gun.visible = false
	camera_yaw = car.rotation.y
	notify("WASD — управление  ·  ПРОБЕЛ — тормоз  ·  E — выйти")

func exit_vehicle(force: bool = false) -> bool:
	if not in_vehicle:
		return false
	if absf(car.speed) > 1.5 and not force:
		notify("Сначала остановите машину")
		return false
	var safe = Vector3.ZERO
	var found: bool = false
	for side in [-1.0, 1.0]:
		var candidate = car.global_position + car.global_basis.x * side * 2.7 + Vector3(0, 0.4, 0)
		var shape_query = PhysicsShapeQueryParameters3D.new()
		shape_query.shape = player.collision.shape
		shape_query.transform = Transform3D(Basis.IDENTITY, candidate + Vector3(0, 0.97, 0))
		shape_query.exclude = [player.get_rid(), car.get_rid()]
		if get_world_3d().direct_space_state.intersect_shape(shape_query, 1).is_empty():
			safe = candidate
			found = true
			break
	if not found and not force:
		notify("Выход заблокирован. Переставьте автомобиль")
		return false
	in_vehicle = false
	car.driven = false
	car.speed = 0
	player.global_position = safe if found and not force else Vector3(0, 0.4, 19)
	player.visible = true
	player.collision.disabled = false
	player.velocity = Vector3.ZERO
	return true

func fire_bullet(origin: Vector3, direction: Vector3) -> void:
	for i in bullet_nodes.size():
		if not bullet_nodes[i].visible:
			bullet_nodes[i].visible = true
			bullet_nodes[i].global_position = origin
			bullets.append({"slot": i, "position": origin, "velocity": direction * 180.0, "age": 0.0})
			break
	for npc in get_tree().get_nodes_in_group("citizens"):
		npc.react_to_noise(origin)
	danger = 8.0

func respawn(message: String) -> void:
	if in_vehicle:
		exit_vehicle(true)
	player.global_position = Vector3(0, 0.5, 19)
	player.velocity = Vector3.ZERO
	player.health = 100
	money = maxi(0, money - 25)
	notify(message)

func notify(message: String) -> void:
	toast = message
	toast_timer = 4.5

func set_mode(value: String) -> void:
	mode = value
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED if value == "play" else Input.MOUSE_MODE_VISIBLE
	if hud:
		hud.rebuild_menu()

func new_game() -> void:
	quest = 0
	step = 0
	money = 150
	inventory.clear()
	hour = 16.2
	weather = 0
	weather_timer = 0
	if in_vehicle:
		exit_vehicle(true)
	player.health = 100
	player.magazine = 15
	player.reserve = 120
	player.global_position = Vector3(0, 0.3, 19)
	car.global_position = Vector3(12, 0.5, 3)
	car.rotation.y = -PI * 0.5
	car.health = 100
	car.speed = 0
	start_session()
	notify("Добро пожаловать. Диспетчер ждёт слева от площади")

func start_session() -> void:
	session_started = true
	set_mode("play")
	if DisplayServer.get_name() != "headless":
		if not music.playing:
			music.play()
		if not ambient.playing:
			ambient.play()

func save_game(show_message: bool = true) -> bool:
	var p = focus_position() + (car.global_basis.x * 2.8 if in_vehicle else Vector3.ZERO)
	p.y = maxf(p.y, 0.3)
	var data = {"quest": quest, "step": step, "money": money, "health": maxf(player.health, 1), "magazine": player.magazine, "reserve": player.reserve, "hour": hour, "weather": weather, "position": [p.x, p.y, p.z], "car_position": [car.global_position.x, maxf(car.global_position.y, 0.15), car.global_position.z], "car_angle": car.rotation.y, "car_health": car.health}
	var success = save_store.save(data)
	if show_message:
		notify("Сохранено" if success else "Сохранение недоступно в этой позиции")
	return success

func load_game() -> bool:
	var data = save_store.load_latest()
	if data.is_empty():
		notify("Нет корректного сохранения")
		return false
	if in_vehicle:
		exit_vehicle(true)
	quest = int(data.quest)
	step = int(data.step)
	money = int(data.money)
	player.health = data.health
	player.magazine = int(data.magazine)
	player.reserve = int(data.reserve)
	player.reload_clock = 0
	player.velocity = Vector3.ZERO
	player.global_position = Vector3(data.position[0], data.position[1], data.position[2])
	car.global_position = Vector3(data.car_position[0], data.car_position[1], data.car_position[2])
	car.rotation.y = data.car_angle
	car.speed = 0
	car.health = data.car_health
	hour = data.hour
	weather = int(data.weather)
	inventory.clear()
	if quest < quest_data.size():
		for i in step:
			var entry = quest_data[quest].steps[i]
			if entry.has("item"):
				inventory.append(entry.item)
	for node in bullet_nodes:
		node.visible = false
	bullets.clear()
	camera_pivot.position = player.position + Vector3.UP * 1.6
	start_session()
	notify("Сохранение загружено")
	return true

func load_settings() -> void:
	var config = ConfigFile.new()
	if config.load("user://settings.cfg") == OK:
		quality = clampi(int(config.get_value("video", "quality", 1)), 0, 2)
		volume = clampf(float(config.get_value("audio", "volume", 0.65)), 0, 1)
		fullscreen = bool(config.get_value("video", "fullscreen", false))

func apply_quality() -> void:
	get_viewport().msaa_3d = [Viewport.MSAA_DISABLED, Viewport.MSAA_2X, Viewport.MSAA_4X][quality]
	get_viewport().scaling_3d_scale = [0.75, 0.9, 1.0][quality]
	sun.shadow_enabled = quality > 0
	for node in builder.vegetation:
		node.visibility_range_end = [150.0, 230.0, 310.0][quality]
	AudioServer.set_bus_volume_db(0, linear_to_db(maxf(volume, 0.001)))
	Engine.max_fps = 60
	if DisplayServer.get_name() != "headless":
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN if fullscreen else DisplayServer.WINDOW_MODE_WINDOWED)
	var config = ConfigFile.new()
	config.set_value("video", "quality", quality)
	config.set_value("video", "fullscreen", fullscreen)
	config.set_value("audio", "volume", volume)
	config.save("user://settings.cfg")
