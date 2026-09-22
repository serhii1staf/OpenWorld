extends CharacterBody3D

const Visuals = preload("res://scripts/visuals.gd")
var game: Node3D
var speed: float = 0.0
var health: float = 100.0
var color: Color = Color("568e91")
var ai: bool = false
var route: Array = []
var route_index: int = 0
var driven: bool = false
var engine_audio: AudioStreamPlayer3D
var headlamps: Array = []
var crash_clock: float = 0.0
var decision_timer: float = 0.0
var ai_brake: bool = false

func _ready() -> void:
	var collider = CollisionShape3D.new()
	var box = BoxShape3D.new()
	box.size = Vector3(1.82, 1.1, 3.8)
	collider.shape = box
	collider.position.y = 0.65
	add_child(collider)
	Visuals.car_model(self, color)
	floor_snap_length = 0.7
	engine_audio = AudioStreamPlayer3D.new()
	engine_audio.stream = load("res://assets/engine.wav")
	engine_audio.max_distance = 45
	engine_audio.volume_db = -14
	add_child(engine_audio)
	for x in [-0.6, 0.6]:
		var light = SpotLight3D.new()
		light.position = Vector3(x, 0.95, -1.9)
		light.spot_range = 26.0
		light.spot_angle = 26.0
		light.light_color = Color("ffe4b0")
		light.light_energy = 1.2
		light.shadow_enabled = false
		add_child(light)
		headlamps.append(light)
	add_to_group("vehicles")

func _physics_process(delta: float) -> void:
	if not game or game.mode != "play":
		return
	crash_clock = maxf(0.0, crash_clock - delta)
	var throttle: float = 0.0
	var steering: float = 0.0
	var braking: bool = false
	if driven:
		throttle = Input.get_axis("back", "forward")
		steering = Input.get_axis("right", "left")
		braking = Input.is_action_pressed("jump")
	elif ai and route.size() > 0:
		var target: Vector3 = route[route_index]
		var offset = target - global_position
		if Vector2(offset.x, offset.z).length() < 6.0:
			route_index = (route_index + 1) % route.size()
			return
		var desired_angle = atan2(-offset.x, -offset.z)
		var turn_error = angle_difference(rotation.y, desired_angle)
		steering = clampf(turn_error * 1.8, -1.0, 1.0)
		decision_timer -= delta
		if decision_timer <= 0.0:
			decision_timer = 0.2
			var origin = global_position + Vector3(0, 0.9, 0) - global_basis.z * 2.2
			var query = PhysicsRayQueryParameters3D.create(origin, origin - global_basis.z * (6.0 + absf(speed)), 1, [get_rid()])
			ai_brake = not get_world_3d().direct_space_state.intersect_ray(query).is_empty()
			if absf(global_position.z) < 14 and absf(absf(global_position.x) - 100) < 10 and int(game.elapsed / 12.0) % 2 == 0:
				ai_brake = true
			var corner_slowdown = clampf(1.0 - absf(turn_error) / 1.25, 0.28, 1.0)
			throttle = 0.65 * corner_slowdown if speed < 10.0 and not ai_brake else 0.0
		braking = ai_brake or (absf(turn_error) > 1.0 and speed > 5.0)
	if health <= 0.0:
		throttle = 0.0
		braking = true
	var wet_grip = 1.0 - game.rain_strength * 0.16
	if braking:
		speed = move_toward(speed, 0.0, delta * 20.0 * wet_grip)
	elif absf(throttle) > 0.0:
		speed = clampf(speed + throttle * delta * 8.0 * wet_grip, -8.0, 24.0)
	else:
		speed = move_toward(speed, 0.0, delta * 2.5)
	if absf(speed) > 0.2:
		rotation.y += steering * delta * clampf(absf(speed) / 5.0, 0.0, 1.0) * 1.25 * signf(speed) * wet_grip
	var forward = -global_basis.z
	velocity.x = forward.x * speed
	velocity.z = forward.z * speed
	velocity.y -= 22.0 * delta
	var previous_speed = speed
	move_and_slide()
	if is_on_floor():
		velocity.y = 0.0
	for i in get_slide_collision_count():
		var hit = get_slide_collision(i)
		if absf(hit.get_normal().y) < 0.5 and absf(previous_speed) > 3.0 and crash_clock <= 0.0:
			crash_clock = 0.7
			if hit.get_collider().has_method("take_hit"):
				hit.get_collider().take_hit(absf(speed) * 2.0, global_position)
			if driven:
				health = maxf(0.0, health - absf(speed) * 0.65)
				game.camera_shake = 0.2
				game.sound_effect("impact", global_position, 0.8)
			speed *= -0.2
	engine_audio.pitch_scale = 0.65 + absf(speed) / 22.0
	var active_engine = driven or ai
	if active_engine and not engine_audio.playing and DisplayServer.get_name() != "headless":
		engine_audio.play()
	elif not active_engine and engine_audio.playing:
		engine_audio.stop()
	for lamp in headlamps:
		lamp.visible = active_engine and (game.hour >= 18.0 or game.hour < 7.0 or game.rain_strength > 0.3)
	if global_position.y < -5:
		if driven:
			game.exit_vehicle(true)
		global_position = Vector3(12, 1, 3)
		speed = 0.0
		health = 100.0

func take_hit(amount: float, _source: Vector3 = Vector3.ZERO) -> void:
	health = maxf(0.0, health - amount)

func _exit_tree() -> void:
	if is_instance_valid(engine_audio):
		engine_audio.stop()
		engine_audio.stream = null
