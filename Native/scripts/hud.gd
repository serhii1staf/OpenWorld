extends Control

var game: Node3D
var font: Font
var menu: VBoxContainer
var menu_panel: PanelContainer
var background_style: StyleBoxFlat
const INK = Color("e5eadf")
const DIM = Color("91aca7")
const GOLD = Color("f3c274")
const TEAL = Color("71c6b0")

func _ready() -> void:
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	font = ThemeDB.fallback_font
	background_style = StyleBoxFlat.new()
	background_style.bg_color = Color(0.025, 0.073, 0.085, 0.9)
	background_style.corner_radius_top_left = 8
	background_style.corner_radius_top_right = 8
	background_style.corner_radius_bottom_left = 8
	background_style.corner_radius_bottom_right = 8
	resized.connect(rebuild_menu)
	rebuild_menu()

func text(value: String, pos: Vector2, size_value: int = 18, color: Color = INK, width: float = -1.0) -> void:
	draw_string(font, pos, value, HORIZONTAL_ALIGNMENT_LEFT, width, size_value, color)

func panel(rect: Rect2) -> void:
	draw_style_box(background_style, rect)

func _draw() -> void:
	if not game or not font:
		return
	var s = maxf(size.y / 900.0, 0.5)
	draw_set_transform(Vector2.ZERO, 0, Vector2(s, s))
	var w = size.x / s
	var h = size.y / s
	if game.mode == "title":
		draw_rect(Rect2(0, 0, w, h), Color(0.015, 0.04, 0.05, 0.55))
		text("O P E N W O R L D", Vector2(66, 118), 22, TEAL)
		text("PORT", Vector2(60, 270), 82)
		text("MERIDIAN", Vector2(60, 357), 82)
		draw_line(Vector2(66, 396), Vector2(270, 396), GOLD, 3)
		text("Один город. Три заказа. Твой маршрут.", Vector2(66, 445), 23)
		text("Исследуй порт, отвези груз и восстанови связь на хребте.", Vector2(66, 484), 17, DIM)
		text("Нативная Windows-альфа  /  v0.2.0", Vector2(66, h - 65), 16, DIM)
		text("Оригинальный небольшой мир. Не AAA и не GTA.", Vector2(66, h - 39), 14, DIM)
		return
	if game.mode in ["pause", "settings", "dialogue"]:
		draw_rect(Rect2(0, 0, w, h), Color(0.015, 0.04, 0.05, 0.74))
		text("OPENWORLD  /  PORT MERIDIAN", Vector2(42, 58), 21, TEAL)
		if game.mode != "dialogue":
			text("ПАУЗА" if game.mode == "pause" else "НАСТРОЙКИ", Vector2(42, 118), 38)
		return
	if game.mode == "map":
		draw_rect(Rect2(0, 0, w, h), Color(0.015, 0.04, 0.05, 0.95))
		text("МЕРИДИАН / КАРТА РАЙОНА", Vector2(45, 60), 28, INK)
		var rect = Rect2((w - 790) * 0.5, 100, 790, 650)
		draw_map(rect, true)
		text("Золотой маркер — текущий заказ    ·    M / ESC — вернуться", Vector2(45, h - 50), 20, DIM)
		return
	panel(Rect2(30, 30, 480, 126))
	text("MERIDIAN / КУРЬЕРСКАЯ СЛУЖБА", Vector2(48, 58), 13, TEAL)
	if game.quest < game.quest_data.size():
		text(game.quest_data[game.quest].name, Vector2(48, 92), 23)
		text(game.quest_data[game.quest].steps[game.step].task, Vector2(48, 124), 16, INK, 445)
	else:
		text("Все заказы выполнены", Vector2(48, 94), 24, GOLD)
		text("Свободное исследование. Спасибо за игру!", Vector2(48, 125), 16, DIM)
	draw_map(Rect2(w - 267, 30, 235, 193), false)
	var minutes = int((game.hour - floorf(game.hour)) * 60)
	panel(Rect2(w - 267, 230, 235, 70))
	text("%02d:%02d   /   %s" % [int(game.hour), minutes, ["Ясно", "Туман", "Дождь"][game.weather]], Vector2(w - 256, 249), 17, INK)
	text("$ %s" % game.money, Vector2(w - 254, 284), 28, GOLD)
	panel(Rect2(30, h - 118, 290, 80))
	text("КУРЬЕР" if not game.in_vehicle else "АВТОМОБИЛЬ", Vector2(48, h - 89), 13, DIM)
	var health = game.car.health if game.in_vehicle else game.player.health
	draw_rect(Rect2(48, h - 72, 218, 5), Color("314b4d"))
	draw_rect(Rect2(48, h - 72, 218 * health / 100.0, 5), TEAL if health > 35 else Color("ec8d6d"))
	if game.in_vehicle:
		text("%02d км/ч" % int(absf(game.car.speed) * 3.6), Vector2(48, h - 45), 20, INK)
	else:
		text("%02d / %03d" % [game.player.magazine, game.player.reserve] if game.player.weapon_visible else "Q  /  оружие убрано", Vector2(48, h - 45), 18, INK)
		if game.player.reload_clock > 0:
			text("ПЕРЕЗАРЯДКА", Vector2(w * 0.5 - 74, h * 0.57), 16, GOLD)
	if not game.inventory.is_empty():
		text("В сумке: " + str(game.inventory[0]), Vector2(32, h - 137), 16, GOLD)
	panel(Rect2(w * 0.5 - 240, h - 46, 500, 33))
	text("E действие  ·  M карта  ·  F5 сохранить  ·  ESC меню", Vector2(w * 0.5 - 225, h - 24), 14, DIM)
	if game.in_vehicle:
		text("WASD езда  /  ПРОБЕЛ тормоз  /  E выход", Vector2(w * 0.5 - 196, h - 53), 16, INK)
	elif not game.nearest_id.is_empty():
		var prompt = "Сесть за руль" if game.nearest_id == "vehicle" else game.markers[game.nearest_id].title
		panel(Rect2(w * 0.5 - 180, h * 0.7, 360, 49))
		text("[ E ]  " + prompt, Vector2(w * 0.5 - 158, h * 0.7 + 32), 20, GOLD)
	if game.player.weapon_visible and not game.in_vehicle:
		draw_circle(Vector2(w * 0.5, h * 0.5), 2.4, INK)
		draw_arc(Vector2(w * 0.5, h * 0.5), 11, 0, TAU, 24, Color(1, 1, 1, 0.4), 1.0)
	if game.quest < game.quest_data.size():
		var destination: Vector3 = game.markers[game.current_target()].position + Vector3.UP * 3.2
		if not game.camera.is_position_behind(destination):
			var screen = game.camera.unproject_position(destination) / s
			if screen.x > 20 and screen.x < w - 20 and screen.y > 170 and screen.y < h - 100:
				draw_circle(screen, 5, GOLD)
				text(str(int(game.focus_position().distance_to(destination))) + " м", screen + Vector2(12, 5), 16, GOLD)
	if game.toast_timer > 0:
		panel(Rect2(w * 0.5 - 335, 178, 670, 47))
		text(game.toast, Vector2(w * 0.5 - 316, 208), 17, GOLD, 635)
	if game.danger > 0:
		text("ОПАСНОСТЬ / жители ищут укрытие", Vector2(32, 185), 16, Color("f0a076"))
	if game.show_stats:
		text("FPS %d  ·  Draw calls %d  ·  Objects %d" % [Engine.get_frames_per_second(), Performance.get_monitor(Performance.RENDER_TOTAL_DRAW_CALLS_IN_FRAME), Performance.get_monitor(Performance.OBJECT_NODE_COUNT)], Vector2(32, h - 165), 15, TEAL)

func draw_map(rect: Rect2, large: bool) -> void:
	panel(rect)
	var inset = rect.grow(-12)
	var project = func(pos: Vector3) -> Vector2:
		return inset.position + Vector2((pos.x + 260) / 520.0, (pos.z + 285) / 505.0) * inset.size
	var ocean_top = project.call(Vector3(0, 0, 180)).y
	draw_rect(Rect2(inset.position.x, ocean_top, inset.size.x, inset.end.y - ocean_top), Color("1f4953"))
	var forest_end = project.call(Vector3(0, 0, -110)).y
	draw_rect(Rect2(inset.position, Vector2(inset.size.x, forest_end - inset.position.y)), Color("25473e"))
	for z in [-100, 0, 100, 150]:
		draw_line(project.call(Vector3(-244, 0, z)), project.call(Vector3(244, 0, z)), Color("6c8074"), 3.0 if large else 1.4)
	for x in [-100, 100]:
		draw_line(project.call(Vector3(x, 0, -278)), project.call(Vector3(x, 0, 175)), Color("6c8074"), 3.0 if large else 1.4)
	for id in game.markers:
		var p: Vector2 = project.call(game.markers[id].position)
		var selected = id == game.current_target()
		draw_circle(p, 6.0 if selected else 3.0, GOLD if selected else DIM)
		if selected:
			draw_arc(p, 10, 0, TAU, 24, GOLD, 1.3)
		if large:
			text(game.markers[id].title, p + Vector2(10, -8), 15, GOLD if selected else INK)
	var car_point: Vector2 = project.call(game.car.position)
	draw_rect(Rect2(car_point - Vector2(3, 3), Vector2(6, 6)), TEAL)
	var dot: Vector2 = project.call(game.focus_position())
	draw_circle(dot, 4.0, Color.WHITE)
	var heading = Vector2(-sin(game.camera_yaw), -cos(game.camera_yaw))
	draw_line(dot, dot + heading * 10, Color.WHITE, 2.0)
	text("N", inset.position + Vector2(inset.size.x - 14, 15), 13, DIM)
	if large:
		text("ПОРТОВАЯ АКВАТОРИЯ", Vector2(inset.position.x + 22, inset.end.y - 12), 14, DIM)

func button(label: String, callback: Callable) -> Button:
	var b = Button.new()
	b.text = label
	b.custom_minimum_size = Vector2(0, 48)
	b.add_theme_font_size_override("font_size", 19)
	var normal = StyleBoxFlat.new()
	normal.bg_color = Color("193a40")
	normal.content_margin_left = 18
	normal.content_margin_right = 18
	normal.corner_radius_top_left = 5
	normal.corner_radius_top_right = 5
	normal.corner_radius_bottom_left = 5
	normal.corner_radius_bottom_right = 5
	var hover = normal.duplicate()
	hover.bg_color = Color("2d5556")
	var focus = normal.duplicate()
	focus.border_color = GOLD
	focus.set_border_width_all(1)
	b.add_theme_stylebox_override("normal", normal)
	b.add_theme_stylebox_override("hover", hover)
	b.add_theme_stylebox_override("pressed", hover)
	b.add_theme_stylebox_override("focus", focus)
	b.pressed.connect(callback)
	menu.add_child(b)
	return b

func label(value: String, font_size: int = 20, min_height: float = 0.0) -> Label:
	var node = Label.new()
	node.text = value
	node.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	node.custom_minimum_size.y = min_height
	node.add_theme_font_size_override("font_size", font_size)
	node.add_theme_color_override("font_color", INK)
	menu.add_child(node)
	return node

func rebuild_menu() -> void:
	if not is_inside_tree() or not game:
		return
	if is_instance_valid(menu_panel):
		menu_panel.queue_free()
		menu_panel = null
	if game.mode in ["play", "map"]:
		return
	menu_panel = PanelContainer.new()
	var style = StyleBoxFlat.new()
	style.bg_color = Color(0.02, 0.06, 0.07, 0.92)
	style.content_margin_left = 24
	style.content_margin_right = 24
	style.content_margin_top = 22
	style.content_margin_bottom = 22
	menu_panel.add_theme_stylebox_override("panel", style)
	add_child(menu_panel)
	menu = VBoxContainer.new()
	menu.add_theme_constant_override("separation", 12)
	menu_panel.add_child(menu)
	var desired_width = 630 if game.mode == "dialogue" else 350
	var actual_width = minf(desired_width, maxf(300, size.x - 50))
	menu_panel.size.x = actual_width
	menu_panel.position = Vector2((size.x - actual_width) * 0.5, size.y * 0.29)
	if game.mode == "title":
		menu_panel.position = Vector2(size.x - actual_width - 58, size.y * 0.46)
		button("НАЧАТЬ МАРШРУТ", game.new_game)
		button("Продолжить сохранение", game.load_game).disabled = game.save_store.load_latest().is_empty()
		button("Настройки", func(): game.set_mode("settings"))
		button("Выйти", func(): get_tree().quit())
	elif game.mode == "pause":
		button("Продолжить", func(): game.set_mode("play"))
		button("Сохранить игру", func(): game.save_game(); game.set_mode("play"))
		button("Загрузить игру", game.load_game)
		button("Настройки", func(): game.set_mode("settings"))
		button("Вернуться на площадь", func(): game.respawn("Возвращение на площадь. −$25"); game.set_mode("play"))
		button("Выйти", func(): game.save_game(false); get_tree().quit())
	elif game.mode == "settings":
		label("Графика и звук", 25)
		button("Качество: " + ["Низкое", "Среднее", "Высокое"][game.quality], func(): game.quality = (game.quality + 1) % 3; game.apply_quality(); rebuild_menu())
		button("Полный экран: " + ("да" if game.fullscreen else "нет"), func(): game.fullscreen = not game.fullscreen; game.apply_quality(); rebuild_menu())
		label("Громкость", 17)
		var slider = HSlider.new()
		slider.min_value = 0
		slider.max_value = 1
		slider.step = 0.05
		slider.value = game.volume
		slider.custom_minimum_size.y = 28
		slider.value_changed.connect(func(value): game.volume = value; game.apply_quality())
		menu.add_child(slider)
		label("WASD ходьба/езда · мышь камера\nShift бег · Ctrl присесть · Space прыжок/тормоз\nE действие · Q оружие · ЛКМ огонь · R зарядить\nM карта · V камера · F5/F9 запись/загрузка\nT время суток · F6 погода · F3 статистика", 14, 140)
		button("Назад", func(): game.set_mode("pause" if game.session_started else "title"))
	elif game.mode == "dialogue":
		label(game.dialogue_title.to_upper(), 27)
		label(game.dialogue_text, 20, 155)
		button("[ E ] Продолжить", game.close_dialogue)
