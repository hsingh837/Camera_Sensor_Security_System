package avl.util;

public class ConfigLoader {

    // Private constructor to enforce controlled creation
    private ConfigLoader() {
    }

    // Static factory method used by Main.java
    public static ConfigLoader load() {
        System.out.println("[Config] Configuration loaded (stub)");
        return new ConfigLoader();
    }
}
