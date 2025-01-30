#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/jiffies.h>

static struct input_handler my_mouse_handler;

// Custom acceleration function
static int apply_acceleration(int delta, unsigned long time_delta) {
    // Simple acceleration: scale delta based on time delta
    // int sensitivity = 2; // Base sensitivity
    // int acceleration = time_delta / 100; // Adjust this formula as needed
    return delta;
}

// Event processing function
static void my_mouse_event(struct input_handle *handle, unsigned int type,
                           unsigned int code, int value) {
    static unsigned long last_time = 0;
    static int last_x = 0, last_y = 0;

    if (type == EV_REL) {
        unsigned long current_time = jiffies;
        unsigned long time_delta = jiffies_to_msecs(current_time - last_time);

        if (code == REL_X) {
            value = apply_acceleration(value, time_delta);
            last_x = value;
        } else if (code == REL_Y) {
            value = apply_acceleration(value, time_delta);
            last_y = value;
        }

        last_time = current_time;
    }

    // Pass the modified event to the next handler
    input_event(handle->dev, type, code, value);
}

// Match function to identify the mouse device
static bool my_mouse_match(struct input_handler *handler, struct input_dev *dev) {
    // Check if the device is a mouse
    if (test_bit(EV_REL, dev->evbit) &&
        (test_bit(REL_X, dev->relbit) || test_bit(REL_Y, dev->relbit))) {
        return true;
    }
    return false;
}

static int my_mouse_connect(struct input_handler *handler, struct input_dev *dev,
                            const struct input_device_id *id) {
    struct input_handle *handle;
    int error;

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "my_mouse_handler";

    error = input_register_handle(handle);
    if (error)
        goto err_free_handle;

    error = input_open_device(handle);
    if (error)
        goto err_unregister_handle;

    printk("Mouse connected");
    return 0;

err_unregister_handle:
    input_unregister_handle(handle);
err_free_handle:
    kfree(handle);
    return error;
}

// Disconnect function to clean up
static void my_mouse_disconnect(struct input_handle *handle) {
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
    printk("Mouse disconnected");
}

static struct input_handler my_mouse_handler = {
    .event      = my_mouse_event,
    .match      = my_mouse_match,
    .connect    = my_mouse_connect,
    .disconnect = my_mouse_disconnect,
    .name       = "my_mouse_handler",
    .id_table   = NULL, // Match all devices
};

static int __init my_mouse_init(void) {
    return input_register_handler(&my_mouse_handler);
}

static void __exit my_mouse_exit(void) {
    input_unregister_handler(&my_mouse_handler);
}

module_init(my_mouse_init);
module_exit(my_mouse_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mimovnik");
MODULE_DESCRIPTION("Custom mouse acceleration kernel module");
