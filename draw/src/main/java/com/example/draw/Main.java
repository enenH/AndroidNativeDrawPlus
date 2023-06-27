package com.example.draw;

import android.annotation.TargetApi;
import android.graphics.PixelFormat;
import android.os.Build;
import android.view.Display;
import android.view.IRotationWatcher;
import android.view.Surface;
import android.view.SurfaceControl;

import com.example.draw.wrappers.DisplayManager;
import com.example.draw.wrappers.ServiceManager;
import com.example.draw.wrappers.Size;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    static native void native_surfaceChanged(int rotation);

    public static String getClipText() {
        return ServiceManager.getClipboardManager().getText().toString();
    }

    @TargetApi(Build.VERSION_CODES.Q)
    public static Surface getSurface(boolean isHide, boolean isSecure) {
        int width, height;

        DisplayManager displayManager = ServiceManager.getDisplayManager();
        Size size = displayManager.getDisplayInfo(Display.DEFAULT_DISPLAY).getSize();

        if (size.getWidth() > size.getHeight()) {
            width = size.getWidth();
            height = size.getHeight();
        } else {
            width = size.getHeight();
            height = size.getWidth();
        }
        SurfaceControl.Builder builder = new SurfaceControl.Builder();

        builder.setName("enennb");
        builder.setFormat(PixelFormat.RGBA_8888);
        if (Build.VERSION.SDK_INT <= 30) {
            try {
                // 获取 SurfaceControl.Builder 类
                Class<?> builderClass = Class.forName("android.view.SurfaceControl$Builder");
                // 获取 setMetadata 方法
                Method setMetadataMethod = builderClass.getDeclaredMethod("setMetadata", int.class, int.class);
                // 设置可访问性
                setMetadataMethod.setAccessible(true);
                // 调用 setMetadata 方法设置元数据]
                if (isHide && !isSecure)
                    setMetadataMethod.invoke(builder, 2, 441731);
                // 获取 setFlags 方法
                Method setFlagsMethod = builderClass.getDeclaredMethod("setFlags", int.class);
                // 设置可访问性
                setFlagsMethod.setAccessible(true);
                setFlagsMethod.invoke(builder, isSecure ? 0x80 : 0x0);
            } catch (ClassNotFoundException | IllegalAccessException |
                     NoSuchMethodException | InvocationTargetException ignored) {
            }
        } else {
            try {
                // 获取 SurfaceControl.Builder 类
                Class<?> builderClass = Class.forName("android.view.SurfaceControl$Builder");
                // 获取 setFlags 方法
                Method setFlagsMethod = builderClass.getDeclaredMethod("setFlags", int.class);
                // 设置可访问性
                setFlagsMethod.setAccessible(true);
                setFlagsMethod.invoke(builder, isSecure ? 0x80 : isHide ? 0x40 : 0x0);
            } catch (ClassNotFoundException | IllegalAccessException |
                     NoSuchMethodException | InvocationTargetException ignored) {
            }
        }
        int rotation = ServiceManager.getWindowManager().getRotation();
        if (rotation == 1 || rotation == 3) {
            builder.setBufferSize(width, height);
        } else {
            builder.setBufferSize(height, width);
        }
        SurfaceControl surfaceControl = builder.build();

        SurfaceControl.Transaction transaction = new SurfaceControl.Transaction();
        transaction.setLayer(surfaceControl, Integer.MAX_VALUE);
        transaction.apply();
        transaction.close();

        ServiceManager.getWindowManager().registerRotationWatcher(new IRotationWatcher.Stub() {
            @Override
            public void onRotationChanged(int rotation) {
                SurfaceControl.Transaction transaction = new SurfaceControl.Transaction();
                transaction.setLayer(surfaceControl, Integer.MAX_VALUE);
                if (rotation == 1 || rotation == 3) {
                    transaction.setBufferSize(surfaceControl, width, height);
                } else {
                    transaction.setBufferSize(surfaceControl, height, width);
                }
                transaction.apply();
                transaction.close();
                native_surfaceChanged(rotation);
            }
        }, 0);

        return new Surface(surfaceControl);
    }

    public static void main(String[] args) {
        getSurface(false, false);
        getClipText();
        native_surfaceChanged(0);
    }

}
