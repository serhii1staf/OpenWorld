extends RefCounted
const V = preload("res://scripts/visuals.gd")
var root: Node3D
var rng = RandomNumberGenerator.new()
var stone: StandardMaterial3D
var dark: StandardMaterial3D
var wood: StandardMaterial3D
var window_mat: StandardMaterial3D
var vegetation: Array = []

func build(parent: Node3D) -> void:
	root = Node3D.new()
	root.name = "AuthoredDistricts"
	parent.add_child(root)
	rng.seed = 280619
	stone = V.mat("stone", Color("b7b5a6"))
	dark = V.mat("road", Color("34474b"))
	wood = V.mat("wood", Color("877057"))
	window_mat = V.mat("window", Color("bad8d7"), 0.3, 0.3)
	V.box(root, Vector3(570, 3, 485), Vector3(0, -1.5, -60), V.mat("grass", Color("6f8a67")), true).visibility_range_end = 0
	var sea = MeshInstance3D.new()
	var plane = PlaneMesh.new()
	plane.size = Vector2(1200, 700)
	plane.subdivide_width = 100
	plane.subdivide_depth = 60
	sea.mesh = plane
	sea.position = Vector3(0, -0.7, 527.5)
	var water = ShaderMaterial.new()
	water.shader = load("res://assets/water.gdshader")
	sea.material_override = water
	root.add_child(sea)
	roads()
	city()
	harbor()
	forest()
	landmarks()
	street_furniture()
	district_props()
	for x in [-284.0, 284.0]:
		V.box(root, Vector3(1, 8, 485), Vector3(x, 3, -60), stone, true).visible = false
	V.box(root, Vector3(570, 8, 1), Vector3(0, 3, -302), stone, true).visible = false

func roads() -> void:
	var paint = V.mat("roadpaint", Color("d0cda9"))
	var stripes: Array = []
	for z in [0.0, 100.0, 150.0, -100.0]:
		V.box(root, Vector3(490, 0.08, 12), Vector3(0, 0.035, z), dark)
		for side in [-1.0, 1.0]:
			V.box(root, Vector3(490, 0.1, 3.2), Vector3(0, 0.05, z + side * 7.65), stone)
		for x in range(-236, 236, 9):
			stripes.append(Transform3D(Basis.IDENTITY, Vector3(x, 0.085, z)))
	for x in [-100.0, 100.0]:
		V.box(root, Vector3(12, 0.09, 450), Vector3(x, 0.04, -64), dark)
		for side in [-1.0, 1.0]:
			V.box(root, Vector3(3.2, 0.11, 450), Vector3(x + side * 7.65, 0.055, -64), stone)
		for z in range(-276, 154, 9):
			stripes.append(Transform3D(Basis(Vector3.UP, PI * 0.5), Vector3(x, 0.091, z)))
	var stripe = BoxMesh.new()
	stripe.size = Vector3(4.5, 0.01, 0.15)
	V.instances(root, stripe, stripes, paint, 0)
	# Deliberately authored pedestrian crossing locations, not generated street obstacles.
	for intersection in [Vector3(-100, 0, 0), Vector3(100, 0, 0), Vector3(100, 0, 100)]:
		for i in range(-4, 5):
			V.box(root, Vector3(0.75, 0.01, 3.0), intersection + Vector3(i * 1.2, 0.1, 9.7), paint)
			V.box(root, Vector3(3.0, 0.01, 0.75), intersection + Vector3(9.7, 0.1, i * 1.2), paint)
		var signal_material = V.mat("signal", Color("ecae59"), 0, 0.8)
		for side in [-1.0, 1.0]:
			V.cylinder(root, 0.1, 3.2, intersection + Vector3(8.5 * side, 1.6, 9), dark)
			V.box(root, Vector3(0.38, 0.9, 0.28), intersection + Vector3(8.5 * side, 3.0, 9), dark)
			V.sphere(root, 0.12, intersection + Vector3(8.5 * side, 3.23, 9.16), signal_material)
	# Dirt path to the northern ranger lookout.
	V.box(root, Vector3(120, 0.05, 5), Vector3(157, 0.025, -195), V.mat("trail", Color("aa9b74")))

func building(pos: Vector3, width: float, depth: float, floors: int, color: Color, shop: String = "") -> void:
	var group = Node3D.new()
	root.add_child(group)
	group.position = pos
	var height = floors * 3.0 + 0.8
	var plaster = V.mat("wall" + str(color), color)
	V.box(group, Vector3(width, height, depth), Vector3(0, height * 0.5, 0), plaster, true)
	V.box(group, Vector3(width + 0.5, 0.28, depth + 0.5), Vector3(0, height, 0), stone)
	V.box(group, Vector3(width + 0.2, 0.65, depth + 0.2), Vector3(0, 0.325, 0), stone)
	var frames: Array = []
	var glass: Array = []
	var columns = maxi(2, int(width / 3.2))
	for floor in floors:
		for column in columns:
			var x = (float(column) - float(columns - 1) * 0.5) * 2.9
			for z in [-depth * 0.5 - 0.035, depth * 0.5 + 0.035]:
				frames.append(Transform3D(Basis.IDENTITY, Vector3(x, 2.0 + floor * 3.0, z)))
				glass.append(Transform3D(Basis.IDENTITY, Vector3(x, 2.0 + floor * 3.0, z + signf(z) * 0.09)))
	var frame_mesh = BoxMesh.new()
	frame_mesh.size = Vector3(1.6, 1.75, 0.12)
	V.instances(group, frame_mesh, frames, stone)
	var glass_mesh = BoxMesh.new()
	glass_mesh.size = Vector3(1.34, 1.46, 0.08)
	V.instances(group, glass_mesh, glass, window_mat)
	for y in range(1, floors):
		V.box(group, Vector3(width + 0.1, 0.14, depth + 0.1), Vector3(0, 0.65 + y * 3.0, 0), stone)
	if floors <= 2:
		var roof = PrismMesh.new()
		roof.size = Vector3(width + 1.0, 2.0, depth + 1.0)
		var roof_node = MeshInstance3D.new()
		roof_node.mesh = roof
		roof_node.position.y = height + 1.0
		roof_node.material_override = V.mat("roof", Color("665754"))
		roof_node.visibility_range_end = 300
		group.add_child(roof_node)
	else:
		V.box(group, Vector3(2.6, 1.1, 2.2), Vector3(1, height + 0.6, 0), dark)
		V.cylinder(group, 0.8, 1.4, Vector3(-width * 0.25, height + 0.7, 1), stone)
	if not shop.is_empty():
		V.box(group, Vector3(width * 0.8, 0.22, 2.0), Vector3(0, 3.0, depth * 0.5 + 0.7), V.mat("awning", Color("456c66")))
		V.box(group, Vector3(width * 0.84, 1.0, 0.15), Vector3(0, 3.85, depth * 0.5 + 0.12), dark)
		V.sign_text(group, shop, Vector3(0, 3.87, depth * 0.5 + 0.22), 40, Color("f1d9a4"))
		V.box(group, Vector3(1.3, 2.3, 0.11), Vector3(0, 1.2, depth * 0.5 + 0.12), dark)

func city() -> void:
	var colors = [Color("bc9b7e"), Color("829e96"), Color("d0b68e"), Color("b27a65"), Color("9ca3a0")]
	# Civic square; deliberate negative space around the starting hub.
	V.box(root, Vector3(76, 0.1, 47), Vector3(0, 0.05, 35), V.mat("plaza", Color("c0b69c")))
	building(Vector3(-34, 0, 48), 15, 16, 3, colors[1], "PORT OFFICE")
	building(Vector3(29, 0, 50), 16, 16, 2, colors[2], "MERIDIAN CAFE")
	building(Vector3(-120, 0, -52), 24, 20, 3, Color("a6bcad"), "MERIDIAN CLINIC")
	building(Vector3(127, 0, 34), 22, 23, 1, colors[3], "NORTHLINE GARAGE")
	for x in [-205.0, -172.0, -62.0, -28.0, 12.0, 50.0, 155.0, 195.0]:
		for z in [-70.0, -35.0, 70.0]:
			if (x < -150 and z < -40) or (x > 140 and z == 70):
				continue
			var floors = rng.randi_range(2, 5) if x < 120 else rng.randi_range(1, 2)
			building(Vector3(x, 0, z), rng.randf_range(14, 21), rng.randf_range(14, 20), floors, colors[rng.randi_range(0, 4)])
	# Older western blocks with lower rooflines and narrow courtyards.
	for x in [-208.0, -169.0, -135.0]:
		building(Vector3(x, 0, 123), 18, 17, 2, colors[rng.randi_range(0, 4)], "MARKET" if x == -169 else "")
	# High-rise silhouette on the eastern avenue.
	for x in [176.0, 216.0]:
		building(Vector3(x, 0, 36), 22, 27, 8, Color("829693"))

func harbor() -> void:
	var rust = V.mat("rust", Color("bb7853"), 0.15)
	var shipping = [V.mat("containerA", Color("517c78")), V.mat("containerB", Color("ae6c50")), V.mat("containerC", Color("c0a36a"))]
	V.box(root, Vector3(340, 0.13, 20), Vector3(36, 0.06, 169), stone)
	for x in range(-100, 201, 12):
		V.cylinder(root, 0.17, 0.8, Vector3(x, 0.4, 177), dark)
	for i in 14:
		var x = -72 + (i % 7) * 13
		var z = 122.0 + floori(float(i) / 7) * 13
		var material = shipping[i % 3]
		V.box(root, Vector3(10, 3.2, 5), Vector3(x, 1.6, z), material, true)
		for rib in 12:
			V.box(root, Vector3(0.1, 3, 0.1), Vector3(x - 4.7 + rib * 0.84, 1.6, z + 2.56), material)
		V.sign_text(root, "M / " + str(210 + i), Vector3(x, 1.8, z + 2.62), 35, Color("ead8ad"))
	for x in [-52.0, 30.0]:
		V.box(root, Vector3(2.5, 23, 2.5), Vector3(x, 11.5, 171), rust, true)
		V.box(root, Vector3(30, 1.2, 1.3), Vector3(x + 10, 23, 171), rust)
		V.box(root, Vector3(3.5, 3, 3), Vector3(x, 21.2, 170), dark)
		V.cylinder(root, 0.05, 12, Vector3(x + 20, 17, 171), dark)
		V.box(root, Vector3(1, 0.5, 1), Vector3(x + 20, 11, 171), dark)
	# Accessible marina pier and moored low-poly boats.
	V.box(root, Vector3(9, 1, 44), Vector3(133, -0.3, 196), wood, true)
	for z in range(178, 217, 3):
		V.box(root, Vector3(8.8, 0.035, 0.09), Vector3(133, 0.23, z), dark)
	for z in [190.0, 207.0]:
		V.box(root, Vector3(4.4, 0.9, 12), Vector3(144, -0.2, z), V.mat("hull", Color("ddd7bf")))
		V.box(root, Vector3(3, 1.4, 4.5), Vector3(144, 0.7, z + 0.5), dark)
		V.cylinder(root, 0.07, 9, Vector3(144, 4.5, z), stone)
	building(Vector3(146, 0, 130), 17, 16, 2, Color("c4a389"), "HARBOR DISPATCH")
	# Lighthouse is a landmark, not a fake gameplay interior.
	V.cylinder(root, 3.0, 15.0, Vector3(222, 7.5, 165), V.mat("lighthouse", Color("ded7c3")), 2.0)
	V.cylinder(root, 2.4, 1.4, Vector3(222, 15.2, 165), dark)
	V.cylinder(root, 1.7, 1.6, Vector3(222, 16.7, 165), V.mat("beacon", Color("ffe2a2"), 0, 1.6))
	V.cylinder(root, 2.3, 2.0, Vector3(222, 18.2, 165), rust, 0)

func forest() -> void:
	var bark = V.mat("bark", Color("685e46"))
	var needles = V.mat("needles", Color("426b58"))
	var needles_light = V.mat("needlesLight", Color("65836a"))
	for cell_x in range(-4, 5):
		for cell_z in range(-5, -1):
			var cell = Node3D.new()
			root.add_child(cell)
			cell.position = Vector3(cell_x * 55, 0, cell_z * 48)
			var trunks: Array = []
			var leaves: Array = []
			var tops: Array = []
			for i in 13:
				var local = Vector3(rng.randf_range(-25, 25), 0, rng.randf_range(-20, 20))
				var p = cell.position + local
				if absf(absf(p.x) - 100) < 13 or absf(p.z + 100) < 16 or (p.x > 104 and p.z > -203 and p.z < -188) or p.distance_to(Vector3(135, 0, -195)) < 15 or p.distance_to(Vector3(-130, 0, -162)) < 17:
					continue
				var scale_value = rng.randf_range(0.75, 1.5)
				var basis = Basis.IDENTITY.scaled(Vector3.ONE * scale_value)
				trunks.append(Transform3D(basis, local + Vector3.UP * 2 * scale_value))
				leaves.append(Transform3D(basis, local + Vector3.UP * 5 * scale_value))
				tops.append(Transform3D(basis, local + Vector3.UP * 7 * scale_value))
			var trunk = CylinderMesh.new()
			trunk.bottom_radius = 0.25
			trunk.top_radius = 0.17
			trunk.height = 4
			trunk.radial_segments = 6
			var canopy = CylinderMesh.new()
			canopy.bottom_radius = 2.5
			canopy.top_radius = 0
			canopy.height = 6
			canopy.radial_segments = 7
			vegetation.append(V.instances(cell, trunk, trunks, bark, 260))
			vegetation.append(V.instances(cell, canopy, leaves, needles, 260))
			vegetation.append(V.instances(cell, canopy, tops, needles_light, 260))
	# Distant landscape silhouette, built as a low-poly terrain mesh, not hundreds of bodies.
	for i in 14:
		var mountain = CylinderMesh.new()
		mountain.bottom_radius = rng.randf_range(45, 80)
		mountain.top_radius = rng.randf_range(0, 7)
		mountain.height = rng.randf_range(35, 80)
		mountain.radial_segments = 7
		var instance = MeshInstance3D.new()
		instance.mesh = mountain
		instance.position = Vector3(-370 + i * 58, mountain.height * 0.5 - 4, -360 - rng.randf_range(0, 40))
		instance.material_override = V.mat("mountains", Color("657e79"))
		root.add_child(instance)
	building(Vector3(-136, 0, -173), 12, 12, 1, Color("8d8467"), "RANGER STATION")
	# Picnic lookout, reachable on foot or by road.
	V.box(root, Vector3(18, 0.12, 14), Vector3(135, 0.06, -199), wood)
	for x in [128.0, 143.0]:
		V.box(root, Vector3(0.14, 1.1, 12), Vector3(x, 0.6, -199), wood)
	V.sign_text(root, "MERIDIAN RIDGE", Vector3(135, 2.4, -204), 50, Color("ede0bd"))

func landmarks() -> void:
	# Square fountain, benches and repair/fuel point.
	V.cylinder(root, 3.5, 0.4, Vector3(0, 0.2, 36), stone)
	V.cylinder(root, 2.9, 0.3, Vector3(0, 0.5, 36), V.mat("fountain", Color("5b9a9a"), 0.25))
	V.cylinder(root, 0.5, 2.7, Vector3(0, 1.5, 36), stone)
	V.sphere(root, 0.65, Vector3(0, 2.9, 36), V.mat("copper", Color("c19457"), 0.6))
	V.box(root, Vector3(18, 0.35, 11), Vector3(128, 4.5, -125), V.mat("service", Color("bf6f4e")))
	for x in [120.0, 136.0]:
		V.box(root, Vector3(0.4, 4.5, 0.4), Vector3(x, 2.25, -125), stone)
		V.box(root, Vector3(0.9, 1.7, 0.7), Vector3(x, 0.85, -125), dark, true)
	V.sign_text(root, "MERIDIAN / FUEL", Vector3(128, 4.5, -119.4), 38, Color("fbe1a3"))

func street_furniture() -> void:
	var pole = V.mat("lampmetal", Color("3c5759"), 0.35)
	var lit = V.mat("lampglass", Color("ffe2aa"), 0, 1.1)
	for x in range(-220, 221, 28):
		for z in [9.0, 91.0, -91.0]:
			if absf(absf(x) - 100) < 10:
				continue
			V.cylinder(root, 0.085, 5.5, Vector3(x, 2.75, z), pole)
			V.box(root, Vector3(1.6, 0.12, 0.13), Vector3(x + 0.7, 5.5, z), pole)
			V.box(root, Vector3(0.65, 0.13, 0.35), Vector3(x + 1.2, 5.45, z), lit)
	for pos in [Vector3(-11, 0, 32), Vector3(11, 0, 32), Vector3(-38, 0, 18), Vector3(133, 0, -195), Vector3(152, 0, 166)]:
		for z in [-0.15, 0.05, 0.25]:
			V.box(root, Vector3(2.5, 0.1, 0.18), pos + Vector3(0, 0.48, z), wood)
		for x in [-0.9, 0.9]:
			V.box(root, Vector3(0.13, 0.5, 0.4), pos + Vector3(x, 0.25, 0), pole)
		V.box(root, Vector3(2.5, 0.38, 0.1), pos + Vector3(0, 0.89, 0.3), wood)
	# Town vegetation, authored clear of roads and gameplay interactions.
	for x in [-77.0, -43.0, 60.0, 81.0, 161.0, 205.0]:
		for z in [19.0, 88.0]:
			V.box(root, Vector3(4, 0.3, 4), Vector3(x, 0.15, z), stone)
			V.cylinder(root, 0.3, 3.6, Vector3(x, 1.8, z), wood)
			var crown = V.sphere(root, 2.6, Vector3(x, 4.5, z), V.mat("townleaves", Color("668a65")))
			crown.scale.y = 0.9

func district_props() -> void:
	# Small authored details break up the procedural silhouette and give streets a readable scale.
	var curb = V.mat("curb_detail", Color("8e9587"))
	var red = V.mat("postbox", Color("b74f46"), 0.1)
	var teal = V.mat("utility_teal", Color("3d7770"), 0.15)
	for pos in [Vector3(-62, 0, 9), Vector3(62, 0, 91), Vector3(-138, 0, 9), Vector3(138, 0, -91)]:
		V.box(root, Vector3(4.2, 0.16, 0.24), pos + Vector3(0, 0.12, 0), curb)
		V.box(root, Vector3(0.16, 1.0, 0.16), pos + Vector3(-1.7, 0.58, 0), teal)
		V.box(root, Vector3(0.16, 1.0, 0.16), pos + Vector3(1.7, 0.58, 0), teal)
		V.box(root, Vector3(3.9, 0.18, 0.18), pos + Vector3(0, 1.02, 0), teal)
	for pos in [Vector3(-76, 0, 88), Vector3(74, 0, 88), Vector3(-153, 0, -88), Vector3(154, 0, 88), Vector3(214, 0, -86)]:
		V.cylinder(root, 0.16, 1.15, pos + Vector3(0, 0.58, 0), red)
		V.box(root, Vector3(0.5, 0.15, 0.32), pos + Vector3(0, 1.05, -0.08), red)
	for pos in [Vector3(-52, 0, 67), Vector3(52, 0, 67), Vector3(-52, 0, -67), Vector3(52, 0, -67)]:
		V.box(root, Vector3(1.8, 1.2, 0.9), pos + Vector3(0, 0.6, 0), teal, true)
		V.box(root, Vector3(1.45, 0.08, 0.08), pos + Vector3(0, 1.2, -0.47), curb)
	# Warm facade lights make the district readable after sunset.
	var warm = V.mat("facade_light", Color("ffd99a"), 0.0, 1.0)
	for pos in [Vector3(-34, 3.0, 56.2), Vector3(29, 3.0, 58.2), Vector3(-120, 3.0, -41.2), Vector3(127, 2.0, 46.2), Vector3(146, 3.0, 138.2)]:
		V.box(root, Vector3(0.65, 0.28, 0.08), pos, warm)
