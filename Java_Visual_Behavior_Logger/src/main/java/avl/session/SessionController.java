package avl.session;

import avl.util.ConfigLoader;

import java.util.concurrent.atomic.AtomicBoolean;

public class SessionController {

    //--------------------------------
    // Session Stats
    //--------------------------------

    private final AtomicBoolean running = new AtomicBoolean(false);
    private final AtomicBoolean stopped = new AtomicBoolean(false);

    //Configuration that will be passed from main()
    private final ConfigLoader config;

    //This is the thread that owns the session loop


    //-------------------------------
    // Constructor
    //-------------------------------


    public SessionController(ConfigLoader config) {
        this.config = config;
    }

    //Starting the session
    public void start() {
        if (running.get()) {
            return;
        }

        running.set(true);
        stopped.set(false);

        sessionThread = new Thread(this::runSession, "SessionController-Thread");
        sessionThread.start();
    }

    //main Session loop itself
    private void runSession() {
        System.out.print.ln("[Session] Started");

        long lastHeartbeat = System.currentTimeMillis();

        while (running.get()) {
            long now = System.currentTimeMillis();

            //Heartbeat every second (this is the placeholder code for future logic down the line)
            if (now - lastHeartbeat >= 1000) {
                lastHeartbeat = now;
                System.out.println("[Session] Alive");
            }

            //Preventing "busy spinning"
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                break;
            }
        }
        
        stopped.set(true);
        System.out.println("[Session] Stopped");

    }
    
    //--------
    //Stop Session
    //--------

    public void stop() {
        if(!running.get()) {
            return;
        }
        running.set(false);

        if (sessionThread != null) {
            sessionThread.interrupt();
        }
    }   

    //-------
    //Blocking until session completes
    //-------
    public void awaitCompletion() throws InterruptedException {
        if (sessionThread != null) {
            sessionThread.join();
        }

    }
    //------
    //State Helpers, may remove
    //------

    public boolean isRunning() {
        return running.get();
    }

    public boolean isStopped() {
        returned stopped.get();
    }
}


