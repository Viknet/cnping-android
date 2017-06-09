all: app

ndk:
	ndk-build

move_helper:
	$(foreach dir, $(sort $(notdir $(wildcard ./libs/*))), mv ./libs/$(dir)/helper ./libs/$(dir)/libhelper.so;)

apk:
	android update project -p ./ --target 1 --subprojects
	ant debug

app: ndk move_helper apk

install:
	adb -d install -r bin/MainActivity-debug.apk

uninstall:
	adb uninstall com.viknet.cnping

clean:
	rm -rf {bin,gen,libs,obj}
	rm -f proguard-project.txt
	rm -f project.properties
	rm -f build.xml
	rm -f local.properties
	rm -f jni/*.o
