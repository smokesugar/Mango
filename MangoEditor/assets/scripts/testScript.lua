
function Update()
	transform = GetTransform()
	transform.translation.x = math.sin(Time.elapsedTime)
	transform.rotation.z = transform.rotation.z - Time.deltaTime*360
	transform.rotation.z = transform.rotation.z % 360;
	SetTransform(transform)
end