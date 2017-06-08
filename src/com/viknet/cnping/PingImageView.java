package com.viknet.cnping;

import android.content.Context;
import android.util.AttributeSet;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.view.View;
import android.view.View.MeasureSpec;

public class PingImageView extends View {
	private Bitmap mBitmap;

	private static native void drawFrame(Bitmap bitmap);

	public PingImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    protected void onDraw(Canvas canvas) {
    	//FIXME not sure what to do to clear Canvas
        canvas.drawColor(Color.BLACK, PorterDuff.Mode.CLEAR);
        mBitmap.eraseColor(Color.TRANSPARENT);
        
        drawFrame(mBitmap);
        canvas.drawBitmap(mBitmap, 0, 0, null);
        invalidate(); //FIXME UNACCEPTABLE!!!
    }

    protected void onMeasure(final int widthMeasureSpec, final int heightMeasureSpec) {
    	//FIXME changing size not working somehow
    	super.onMeasure(widthMeasureSpec, heightMeasureSpec);
	    final int newHeight= MeasureSpec.getSize(heightMeasureSpec);
	    final int newWidth= MeasureSpec.getSize(widthMeasureSpec);
	    if (mBitmap == null || mBitmap.getWidth() != newWidth || mBitmap.getHeight() != newHeight){
	    	mBitmap = Bitmap.createBitmap(newWidth, newHeight, Bitmap.Config.ARGB_8888);
	    }
	}
}
