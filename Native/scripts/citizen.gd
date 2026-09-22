extends CharacterBody3D

const Visuals = preload("res://scripts/visuals.gd")
var game: Node3D
var home: Vector3
var destination: Vector3
var appearance: int = 0
var model: Node3D
var phase: float = 0.0
var health: float = 60.0
var frightened: float = 0.0
var decision_timer: float = 0.0
var rest_timer: float = 0.0
var direction: Vector3 = Vector3.ZERO
var story_actor: bool = false

func _ready() -> void:
	var collider = CollisionShape3D.new()
	var capsule = CapsuleShape3D.new()
	capsule.radius = 0.32
	capsule.height = 1.9
	collider.shape = capsule
	collider.position.y = 0.96
	add_child(collider)
	var coats = [Color("b96550"), Color("548982"), Color("49617a"), Color("d4b46e"), Color("818362")]
	var skins = [Color("c69978"), Color("805a43"), Color("d9b494")]
	model = Visuals.person(self, coats[appearance % coats.size()], skins[appearance % skins.size()], appearance)
	floor_snap_length = 0.4
	add_to_group("citizens")

func _physics_process(delta: float) -> void:
	if not game or game.mode != "play":
		return
	decision_timer -= delta
	if rest_timer > 0.0 and health > 0.0:
		rest_timer -= delta
		direction = Vector3.ZERO
	if decision_timer <= 0.0:
		decision_timer = 0.4 + float(appearance % 4) * 0.12
		var distance = global_position.distance_squared_to(game.focus_position())
		visible = distance < 180.0 * 180.0
		if distance > 110.0 * 110.0:
			direction = Vector3.ZERO
		elif not story_actor and health > 0.0:
			var target = destination if game.hour > 7 and game.hour < 21 and game.rain_strength < 0.75 else home
			if global_position.distance_to(target) < 2.0:
				var temp = home
				home = destination
				destination = temp
				rest_timer = 1.2 + float(appearance % 3) * 0.35
				direction = Vector3.ZERO
			else:
				direction = (target - global_position).normalized()
			direction.y = 0.0
	if health <= 0.0:
		rest_timer -= delta
		model.rotation.z = lerpf(model.rotation.z, PI * 0.45, delta * 4)
		if rest_timer <= 0:
			health = 60.0
			model.rotation.z = 0.0
			global_position = home
		return
	frightened = maxf(0, frightened - delta)
	if story_actor:
		direction = Vector3.ZERO
	var speed = 4.5 if frightened > 0 else 1.65
	if rest_timer > 0.0:
		speed = 0.0
	velocity.x = direction.x * speed
	velocity.z = direction.z * speed
	velocity.y -= 22.0 * delta
	move_and_slide()
	if get_slide_collision_count() > 0 and not story_actor:
		for i in get_slide_collision_count():
			if absf(get_slide_collision(i).get_normal().y) < 0.5:
				direction = direction.rotated(Vector3.UP, PI / 2)
	phase += delta * speed * 2.2
	Visuals.animate_person(model, phase, direction.length() * speed)
	if direction.length_squared() > 0.01:
		model.rotation.y = lerp_angle(model.rotation.y, atan2(-direction.x, -direction.z), delta * 5.0)

func react_to_noise(source: Vector3) -> void:
	if not story_actor and global_position.distance_to(source) < 35.0:
		frightened = 7.0
		direction = (global_position - source).normalized()
		direction.y = 0

func take_hit(amount: float, source: Vector3 = Vector3.ZERO) -> void:
	if story_actor or health <= 0.0:
		return
	health = maxf(0.0, health - amount)
	react_to_noise(source)
	if health <= 0:
		rest_timer = 22.0
		game.money = maxi(0, game.money - 25)
		game.notify("Помощь пострадавшему: −$25")
