# Check for at least Qt x.y.
# This does the same thing as VersionAtLeast(), but that was only added in
# Qt 5.10, so it's not available to check for 5.2 or 5.8 (for example).
defineTest(checkVersionAtLeast) {
	min_major=$$1
	min_minor=$$2

	# Check major version
	greaterThan(QT_MAJOR_VERSION, $$min_major) {
		return(true)
	}
	lessThan(QT_MAJOR_VERSION, $$min_major) {
		return(false)
	}

	# Check minor version
	lessThan(QT_MINOR_VERSION, $$min_minor) {
		return(false)
	}
}
