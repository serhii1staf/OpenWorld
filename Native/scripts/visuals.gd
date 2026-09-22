extends RefCounted

static var materials: Dictionary = {}

static func mat(id: String, color: Color, metallic: float = 0.0, glow: float = 0.0) -> StandardMaterial3D:
	if materials.has(id):
		return materials[id]
	var m = StandardMaterial3D.new()
	m.albedo_color = color
	m.roughness = 0.8 if metallic == 0.0 else 0.3
	m.metallic = metallic
	if glow > 0.0:
		m.emission_enabled = true
		m.emission = color
		m.emission_energy_multiplier = glow
	materials[id] = m
	return m

static func box(parent: Node3D, size: Vector3, pos: Vector3, material: Material, solid: bool = false) -> MeshInstance3D:
	var mesh = MeshInstance3D.new()
	var shape = BoxMesh.new()
	shape.size = size
	mesh.mesh = shape
	mesh.material_override = material
	mesh.position = pos
	mesh.visibility_range_end = 270.0
	mesh.visibility_range_end_margin = 25.0
	parent.add_child(mesh)
	if solid:
		var body = StaticBody3D.new()
		var collider = CollisionShape3D.new()
		var collision = BoxShape3D.new()
		collision.size = size
		collider.shape = collision
		mesh.add_child(body)
		body.add_child(collider)
	return mesh

static func cylinder(parent: Node3D, radius: float, height: float, pos: Vector3, material: Material, top: float = -1.0) -> MeshInstance3D:
	var mesh = MeshInstance3D.new()
	var shape = CylinderMesh.new()
	shape.bottom_radius = radius
	shape.top_radius = radius if top < 0.0 else top
	shape.height = height
	shape.radial_segments = 24
	mesh.mesh = shape
	mesh.material_override = material
	mesh.position = pos
	mesh.visibility_range_end = 200.0
	parent.add_child(mesh)
	return mesh

static func capsule(parent: Node3D, radius: float, height: float, pos: Vector3, material: Material) -> MeshInstance3D:
	var mesh = MeshInstance3D.new()
	var shape = CapsuleMesh.new()
	shape.radius = radius
	shape.height = height
	shape.radial_segments = 16
	shape.rings = 6
	mesh.mesh = shape
	mesh.material_override = material
	mesh.position = pos
	parent.add_child(mesh)
	return mesh

static func sphere(parent: Node3D, radius: float, pos: Vector3, material: Material) -> MeshInstance3D:
	var mesh = MeshInstance3D.new()
	var shape = SphereMesh.new()
	shape.radius = radius
	shape.height = radius * 2.0
	shape.radial_segments = 24
	shape.rings = 12
	mesh.mesh = shape
	mesh.position = pos
	mesh.material_override = material
	parent.add_child(mesh)
	return mesh

static func sign_text(parent: Node3D, text: String, pos: Vector3, size: int = 48, tint: Color = Color.WHITE) -> Label3D:
	var label = Label3D.new()
	label.text = text
	label.position = pos
	label.font_size = size
	label.pixel_size = 0.018
	label.modulate = tint
	label.outline_size = 0
	label.no_depth_test = false
	label.visibility_range_end = 100.0
	parent.add_child(label)
	return label

static func instances(parent: Node3D, mesh: Mesh, transforms: Array, material: Material, distance: float = 230.0) -> MultiMeshInstance3D:
	var node = MultiMeshInstance3D.new()
	var multi = MultiMesh.new()
	multi.transform_format = MultiMesh.TRANSFORM_3D
	multi.mesh = mesh
	multi.instance_count = transforms.size()
	for i in transforms.size():
		multi.set_instance_transform(i, transforms[i])
	node.multimesh = multi
	node.material_override = material
	node.visibility_range_end = distance
	node.visibility_range_end_margin = 25.0
	parent.add_child(node)
	return node

static func person(parent: Node3D, coat_color: Color, skin_color: Color, seed_value: int = 0) -> Node3D:
	var root = Node3D.new()
	parent.add_child(root)
	var coat = mat("coat" + str(coat_color), coat_color)
	var skin = mat("skin" + str(skin_color), skin_color)
	var navy = mat("navy", Color("263541"))
	var shoes = mat("shoe", Color("182128"))
	var torso = capsule(root, 0.31, 0.82, Vector3(0, 1.22, 0), coat)
	torso.scale = Vector3(1.0, 1.0, 0.72)
	box(root, Vector3(0.43, 0.19, 0.3), Vector3(0, 0.87, 0), navy)
	cylinder(root, 0.09, 0.16, Vector3(0, 1.61, 0), skin)
	var head = sphere(root, 0.23, Vector3(0, 1.85, 0), skin)
	head.scale = Vector3(0.88, 1.18, 0.91)
	var hair = sphere(root, 0.233, Vector3(0, 1.98, 0.035), mat("hair" + str(seed_value % 3), [Color("252324"), Color("594231"), Color("b18449")][seed_value % 3]))
	hair.scale = Vector3(0.93, 0.64, 0.89)
	var face = mat("face" + str(seed_value % 3), Color("362d2a"))
	for side in [-1.0, 1.0]:
		sphere(root, 0.025, Vector3(side * 0.085, 1.86, -0.216), face)
	for side in [-1.0, 1.0]:
		box(root, Vector3(0.037, 0.034, 0.027), Vector3(side * 0.085, 1.86, -0.192), shoes)
		var arm = Node3D.new()
		arm.name = "ArmL" if side < 0.0 else "ArmR"
		root.add_child(arm)
		arm.position = Vector3(side * 0.35, 1.48, 0)
		capsule(arm, 0.105, 0.56, Vector3(0, -0.28, 0), coat)
		sphere(arm, 0.105, Vector3(0, -0.59, 0), skin)
		var leg = Node3D.new()
		leg.name = "LegL" if side < 0.0 else "LegR"
		root.add_child(leg)
		leg.position = Vector3(side * 0.15, 0.85, 0)
		capsule(leg, 0.13, 0.72, Vector3(0, -0.36, 0), navy)
		box(leg, Vector3(0.24, 0.16, 0.39), Vector3(0, -0.76, -0.065), shoes)
	return root

static func animate_person(root: Node3D, phase: float, speed: float) -> void:
	var amount = clampf(speed / 4.0, 0.0, 1.0)
	root.get_node("LegL").rotation.x = sin(phase) * 0.65 * amount
	root.get_node("LegR").rotation.x = -sin(phase) * 0.65 * amount
	root.get_node("ArmL").rotation.x = -sin(phase) * 0.48 * amount
	root.get_node("ArmR").rotation.x = sin(phase) * 0.48 * amount

static func car_model(parent: Node3D, color: Color) -> Node3D:
	var root = Node3D.new()
	parent.add_child(root)
	var paint = mat("car" + str(color), color, 0.35)
	var glass = mat("glass", Color("24404b"), 0.45)
	var dark = mat("rubber", Color("14212a"))
	var steel = mat("steel", Color("a1b2b4"), 0.65)
	box(root, Vector3(1.78, 0.59, 3.9), Vector3(0, 0.81, 0), paint)
	box(root, Vector3(1.61, 0.72, 1.91), Vector3(0, 1.39, 0.16), paint)
	box(root, Vector3(1.47, 0.47, 0.04), Vector3(0, 1.42, -0.81), glass).rotation.x = -0.15
	box(root, Vector3(1.47, 0.46, 0.04), Vector3(0, 1.42, 1.14), glass).rotation.x = 0.16
	for x in [-0.814, 0.814]:
		box(root, Vector3(0.025, 0.47, 1.52), Vector3(x, 1.42, 0.15), glass)
		box(root, Vector3(0.07, 0.62, 0.085), Vector3(x, 1.42, 0.25), paint)
		box(root, Vector3(0.08, 0.04, 0.25), Vector3(x, 1.05, 0.3), steel)
	for z in [-1.22, 1.24]:
		for x in [-0.93, 0.93]:
			var wheel = cylinder(root, 0.45, 0.24, Vector3(x, 0.55, z), dark)
			wheel.rotation.z = PI * 0.5
			var hub = cylinder(root, 0.25, 0.26, Vector3(x, 0.55, z), steel)
			hub.rotation.z = PI * 0.5
	box(root, Vector3(1.8, 0.15, 0.16), Vector3(0, 0.63, -1.98), steel)
	box(root, Vector3(1.8, 0.15, 0.16), Vector3(0, 0.63, 1.98), dark)
	for x in [-0.59, 0.59]:
		box(root, Vector3(0.43, 0.22, 0.05), Vector3(x, 0.9, -1.96), mat("headlights", Color("ffe5ad"), 0.0, 1.2))
		box(root, Vector3(0.34, 0.18, 0.05), Vector3(x, 0.92, 1.96), mat("taillights", Color("f44b42"), 0.0, 0.6))
	return root
