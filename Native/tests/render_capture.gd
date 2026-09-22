extends SceneTree
func _initialize() -> void:
	call_deferred("run")
func run() -> void:
	var game = load("res://main.tscn").instantiate()
	root.add_child(game)
	root.size = Vector2i(1440, 900)
	for i in 6:
		await process_frame
	await RenderingServer.frame_post_draw
	root.get_texture().get_image().save_png("/home/user/.cache/godot/title.png")
	game.new_game()
	for i in 12:
		await process_frame
	await RenderingServer.frame_post_draw
	root.get_texture().get_image().save_png("/home/user/.cache/godot/gameplay.png")
	game.player.global_position = Vector3(133, 0.3, 167)
	game.camera_yaw = -0.6
	game.camera_pitch = -0.25
	for i in 15:
		await process_frame
	await RenderingServer.frame_post_draw
	root.get_texture().get_image().save_png("/home/user/.cache/godot/harbor.png")
	print("RENDER_CAPTURE_OK")
	quit()
