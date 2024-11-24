# haze_detect_app_src

更新日志：

1. 本版引入了新依赖，以适应百分比布局

```
dependencies {
    implementation fileTree(dir: 'libs', include: ['*.jar'])
    implementation "org.jetbrains.kotlin:kotlin-stdlib-jdk7:$kotlin_version"
    implementation 'androidx.appcompat:appcompat:1.4.1'
    implementation 'androidx.constraintlayout:constraintlayout:2.1.3'
    implementation 'com.google.android.gms:play-services-basement:17.6.0'
    implementation 'com.google.android.gms:play-services-base:17.6.0'
    implementation 'com.google.android.gms:play-services-location:18.0.0'
    implementation 'com.google.android.gms:play-services-tasks:17.2.1'
    implementation 'com.google.android.material:material:1.4.0'
    implementation 'com.squareup.okhttp3:okhttp:3.4.1'
    implementation 'androidx.percentlayout:percentlayout:1.0.0'				//此处
    testImplementation 'junit:junit:4.12'
    androidTestImplementation 'androidx.test.ext:junit:1.1.3'
    androidTestImplementation 'androidx.test.espresso:espresso-core:3.4.0'
    implementation project(path: ':opencv')
}
```

2. 更新了activity结构，单一activity多功能。
