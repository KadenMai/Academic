/*********************************************************/
/* Student: Khanh Mai*/
/* Project: Merging Sort List in Java*/
/*********************************************************/
package mergingsort;
import java.util.*;

public class Application implements Runnable{
    public static final int n = 2000000;
    private int[] XY;
    private int[] Z;
    private int offset;
    
    public Application(int[] XY, int[] Z, int offset) {
        super();
        this.XY = XY;
        this.Z = Z;
        this.offset = offset;
    }

    @Override
    public void run() {
        for(int i = 0; i < n; i++) {
            distribute(i + offset, XY, Z);
        }
    }
    
    public static void distribute(int index, 
            int[] XY, int[] Z){
        int item = XY[index];
        int head = -1;
        int tail = -1;
        int pivot = -1;
        
        if (index < n) {
            head = n;
            tail = 2*n - 1;
            
            while (head <= tail) {
                pivot = (head + tail)/2;
                if (item <= XY[pivot]) {
                    tail = pivot - 1;
                }
                else {
                    head = ++pivot;
                }
            }
        }
        else {
            head = 0;
            tail = n - 1;
            
            while (head <= tail) {
                pivot = (head + tail)/2;
                if (item < XY[pivot]) {
                    tail = pivot - 1;
                }
                else {
                    head = ++pivot;
                }
            }
        }
        Z[index + pivot - n] = item;
    }
    
    public static void main(String[] args) {
        int i;
        int[] X = new int[n];
        int[] Y = new int[n];
        int[] XY = new int[2*n];
        int[] Z = new int[2*n];
        long seqstart, parstart;

        // INPUT & MERGE X AND Y INTO ONE ARRAY
        for (i = 0; i < n; i++) {
            X[i] = i * 2;
            XY[i] = X[i];
            Y[i] = 1 + i * 2;
            XY[i + n] = Y[i];
        }
        
        // SEQUENTIAL EXECUTION
        Date start = new Date();
        for (i = 0; i < 2*n; i++) {
                distribute(i, XY, Z);
        }
        seqstart = (new Date()).getTime() - start.getTime();
        
        // PARALLEL EXECUTION
        start = new Date();
        Z = new int[2*n];
        Thread threadX = new Thread(new Application(XY, Z, 0));
        Thread threadY = new Thread(new Application(XY, Z, n));
        threadX.start();
        threadY.start();
        try { threadX.join(); threadY.join(); }
        catch(InterruptedException e) {}
        parstart = (new Date()).getTime() - start.getTime();
        
        // STASTICS
        System.out.println("SEQUENTIAL EXECUTION TIME: " + seqstart);
        System.out.println("PARALLEL EXECUTION TIME: " + parstart);
        System.out.println("SPEEDUP: " + (float)seqstart/parstart);
    }
}
