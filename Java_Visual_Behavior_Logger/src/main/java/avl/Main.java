package avl;

import avl.session.SessionController;
import avl.util.ConfigLoader;

public class Main {

    public static void main(String[] args) {

        System.out.println("=== CAMSENS Java Visual Behavior Logger ===");

        // 1. Load configuration (YAML, properties, etc.)
        ConfigLoader config;
        try {
            config = ConfigLoader.load();
        } catch (Exception e) {
            System.err.println("Failed to load configuration.");
            e.printStackTrace();
            return;
        }

        // 2. Create session controller (owns lifecycle)
        SessionController session = new SessionController(config);

        // 3. Add shutdown hook for clean termination (CTRL+C, kill, etc.)
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("Shutdown signal received. Stopping session...");
            session.stop();
        }));

        // 4. Start the session
        try {
            session.start();
        } catch (Exception e) {
            System.err.println("Session failed to start.");
            e.printStackTrace();
            session.stop();
            return;
        }

        // 5. Block until session completes
        try {
            session.awaitCompletion();
        } catch (InterruptedException e) {
            System.err.println("Main thread interrupted.");
            Thread.currentThread().interrupt();
        }

        // 6. Final cleanup
        session.stop();
        System.out.println("Session complete. Exiting.");
    }
}
