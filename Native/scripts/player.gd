extends CharacterBody3D

const Visuals = preload("res://scripts/visuals.gd")
var game: Node3D
var model: Node3D
var health: float = 100.0
var phase: float = 0.0
var crouched: bool = false
var fire_clock: float = 0.0
var reload_clock: float = 0.0
var magazine: int = 15
var reserve: int = 120
var weapon_visible: bool = false
var gun: Node3D
var foot_clock: float = 0.0
var collision: CollisionShape3D

func _ready() -> void:
	collision = CollisionShape3D.new()
	var capsule = CapsuleShape3D.new()
	capsule.radius = 0.32
	capsule.height = 1.9
	collision.shape = capsule
	collision.position.y = 0.96
	add_child(collision)
	floor_snap_length = 0.4
	model = Visuals.person(self, Color("daa65a"), Color("b8886c"), 0)
	var strap = Visuals.box(model, Vector3(0.08, 0.8, 0.03), Vector3(0, 1.23, -0.19), Visuals.mat("strap", Color("383b32")))
	strap.rotation.z = -0.5
	Visuals.box(model, Vector3(0.33, 0.42, 0.21), Vector3(0.36, 0.97, 0.17), Visuals.mat("bag", Color("405d51")))
	gun = Node3D.new()
	model.add_child(gun)
	gun.position = Vector3(0.36, 1.34, -0.34)
	Visuals.box(gun, Vector3(0.09, 0.12, 0.37), Vector3.ZERO, Visuals.mat("gun", Color("263940"), 0.5))
	Visuals.box(gun, Vector3(0.08, 0.19, 0.08), Vector3(0, -0.1, 0.1), Visuals.mat("grip", Color("202528")))
	gun.visible = false
	add_to_group("damageable")

func _physics_process(delta: float) -> void:
	fire_clock = maxf(0.0, fire_clock - delta)
	if reload_clock > 0.0:
		reload_clock -= delta
		if reload_clock <= 0.0:
			var count = mini(15 - magazine, reserve)
			magazine += count
			reserve -= count
	if not game or game.mode != "play" or game.in_vehicle:
		return
	var input = Input.get_vector("left", "right", "forward", "back")
	var basis_yaw = Basis(Vector3.UP, game.camera_yaw)
	var direction = basis_yaw * Vector3(input.x, 0, input.y)
	crouched = Input.is_action_pressed("crouch")
	var speed = 2.1 if crouched else 7.8 if Input.is_action_pressed("sprint") else 4.7
	velocity.x = move_toward(velocity.x, direction.x * speed, 20.0 * delta)
	velocity.z = move_toward(velocity.z, direction.z * speed, 20.0 * delta)
	if not is_on_floor():
		velocity.y -= 22.0 * delta
	elif Input.is_action_just_pressed("jump") and not crouched:
		velocity.y = 7.2
	move_and_slide()
	phase += delta * Vector2(velocity.x, velocity.z).length() * 2.1
	Visuals.animate_person(model, phase, direction.length() * speed)
	model.scale.y = lerpf(model.scale.y, 0.72 if crouched else 1.0, minf(delta * 10.0, 1.0))
	if direction.length_squared() > 0.03:
		model.rotation.y = lerp_angle(model.rotation.y, atan2(-direction.x, -direction.z), delta * 12.0)
	if weapon_visible:
		model.rotation.y = lerp_angle(model.rotation.y, game.camera_yaw, delta * 16.0)
		model.get_node("ArmR").rotation.x = -1.3
	foot_clock -= delta
	if direction.length() > 0.1 and is_on_floor() and foot_clock <= 0.0:
		game.sound_effect("step", global_position, 0.45)
		foot_clock = 0.28 if speed > 5.0 else 0.43
	if global_position.y < -4.0:
		game.respawn("Возвращение на берег. −$25")
	if weapon_visible and Input.is_action_pressed("fire"):
		try_fire()

func try_fire() -> bool:
	if game.mode != "play" or game.in_vehicle or fire_clock > 0.0 or reload_clock > 0.0 or magazine <= 0:
		return false
	magazine -= 1
	fire_clock = 0.22
	var origin = global_position + Vector3(0, 1.5, 0)
	var forward = -game.camera.global_basis.z
	game.fire_bullet(origin + forward * 0.4, forward)
	game.sound_effect("shot", origin, 0.85)
	game.camera_shake = 0.065
	return true

func reload() -> void:
	if reload_clock <= 0.0 and magazine < 15 and reserve > 0:
		reload_clock = 1.3
		game.sound_effect("reload", global_position, 0.7)

func take_hit(amount: float, _source: Vector3 = Vector3.ZERO) -> void:
	if game.mode != "play" or game.in_vehicle:
		return
	health = maxf(0.0, health - amount)
	game.camera_shake = 0.2
	if health <= 0.0:
		game.respawn("Медицинская помощь. −$25")
