import com.android.tools.r8.R8
import java.io.PrintStream
import java.nio.file.Files
import java.nio.file.Paths
import java.util.stream.Collectors

@Suppress("DSL_SCOPE_VIOLATION") // TODO: Remove once KTIJ-19369 is fixed
plugins {
    alias(libs.plugins.androidLibrary)
}

android {
    namespace = "com.example.draw"
    compileSdk = 33

    defaultConfig {
        minSdk = 28
        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17")
                abiFilters("arm64-v8a")
            }
        }
        consumerProguardFiles("consumer-rules.pro")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    buildFeatures {
        aidl = true
    }
    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    ndkVersion = "25.2.9519653"
}
android.libraryVariants.all {
    val jarTask = tasks.register("create${name.capitalize()}MainJar") {

        doLast {
            val classDir = Paths.get(buildDir.path, "intermediates",
                    "javac", this@all.name, "classes")

            val classFiles = Files.walk(classDir)
                    .filter { Files.isRegularFile(it) && it.toString().endsWith(".class") }
                    .collect(Collectors.toList())

            val androidJar = Paths.get(android.sdkDirectory.path, "platforms",
                    android.compileSdkVersion, "android.jar")

            val output = Paths.get(
                    android.sourceSets.getByName("main").assets.srcDirs.first().path, "main.jar")

            if (Files.notExists(output.parent))
                Files.createDirectories(output.parent)

            val pgConf = File(buildDir, "mainJar.pro")

            PrintStream(pgConf.outputStream()).use {
                it.println("-keep class com.example.draw.Main")
                it.println("{ public static void main(java.lang.String[]); }")
            }

            val args = mutableListOf<Any>(
                    "--debug", "--output", output,
                    "--pg-conf", pgConf,
                    "--classpath", androidJar
            ).apply { addAll(classFiles) }
                    .map { it.toString() }
                    .toTypedArray()

            R8.main(args)
        }
    }
    javaCompileProvider {
        finalizedBy(jarTask)
    }
}
dependencies {
}