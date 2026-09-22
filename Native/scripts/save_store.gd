extends RefCounted

var base: String = "user://meridian"
var generation: int = 0
var error: String = ""

func read_slot(path: String) -> Dictionary:
	if not FileAccess.file_exists(path):
		return {}
	var file = FileAccess.open(path, FileAccess.READ)
	if file == null or file.get_length() > 100000:
		return {}
	var parser = JSON.new()
	if parser.parse(file.get_as_text()) != OK:
		return {}
	var parsed = parser.data
	if not parsed is Dictionary or not parsed.get("payload") is String or not parsed.get("hash") is String:
		return {}
	if parsed.payload.sha256_text() != parsed.hash:
		return {}
	if parser.parse(parsed.payload) != OK:
		return {}
	var data = parser.data
	if not data is Dictionary or not validate(data):
		return {}
	return data

func validate(data: Dictionary) -> bool:
	for key in ["schema", "generation", "quest", "step", "money", "health", "magazine", "reserve", "hour", "weather", "car_health", "car_angle"]:
		if not data.has(key) or not (data[key] is float or data[key] is int) or not is_finite(float(data[key])):
			return false
	if data.schema != 1 or data.generation < 0 or data.generation > 1000000000:
		return false
	var counts = [3, 3, 3]
	if int(data.quest) != data.quest or data.quest < 0 or data.quest > 3 or data.step < 0 or int(data.step) != data.step:
		return false
	if data.quest == 3:
		if data.step != 0:
			return false
	elif data.step >= counts[int(data.quest)]:
		return false
	if data.money < 0 or data.money > 999999 or data.health <= 0 or data.health > 100 or data.magazine < 0 or data.magazine > 15 or data.reserve < 0 or data.reserve > 999 or data.hour < 0 or data.hour >= 24 or data.weather < 0 or data.weather > 2 or data.car_health < 0 or data.car_health > 100:
		return false
	for key in ["position", "car_position"]:
		if not data.get(key) is Array or data[key].size() != 3:
			return false
		for value in data[key]:
			if not (value is float or value is int) or not is_finite(float(value)):
				return false
		if absf(data[key][0]) > 280 or data[key][2] < -298 or data[key][2] > 218 or data[key][1] < -1 or data[key][1] > 25:
			return false
	return true

func load_latest() -> Dictionary:
	var a = read_slot(base + "_a.json")
	var b = read_slot(base + "_b.json")
	var latest: Dictionary = b if a.is_empty() or (not b.is_empty() and b.generation > a.generation) else a
	if not latest.is_empty():
		generation = int(latest.generation)
	return latest

func save(data: Dictionary) -> bool:
	error = ""
	var a = read_slot(base + "_a.json")
	var b = read_slot(base + "_b.json")
	var destination = base + "_a.json"
	generation = 0
	if not a.is_empty():
		generation = maxi(generation, int(a.generation))
	if not b.is_empty():
		generation = maxi(generation, int(b.generation))
	if not a.is_empty() and (b.is_empty() or a.generation >= b.generation):
		destination = base + "_b.json"
	data["schema"] = 1
	data["generation"] = generation + 1
	if not validate(data):
		error = "Invalid save snapshot"
		return false
	var payload = JSON.stringify(data)
	var envelope = JSON.stringify({"payload": payload, "hash": payload.sha256_text()})
	var file = FileAccess.open(destination + ".tmp", FileAccess.WRITE)
	if not file:
		error = "Cannot write save file"
		return false
	file.store_string(envelope)
	file.flush()
	file.close()
	if FileAccess.file_exists(destination):
		DirAccess.remove_absolute(destination)
	if DirAccess.rename_absolute(destination + ".tmp", destination) != OK:
		error = "Cannot finalize save file"
		return false
	generation += 1
	return true
