/* swiped from arduino 1.0.6 */

#define makeKeymap(x) ((char*)x)

typedef char KeypadEvent;

typedef enum {
        IDLE=0,
        PRESSED,
        RELEASED,
        HOLD
} KeypadState;

typedef struct {
    byte rows : 4;
    byte columns : 4;
} KeypadSize;

const char NO_KEY = '\0';
#define KEY_RELEASED NO_KEY

class Keypad {
public:
        Keypad(char *userKeymap, byte *row, byte *col, byte rows, byte cols);

        void begin(char *userKeymap) { /* nothing */ }
        char getKey() { return( NO_KEY ); }
        KeypadState getState() { return( IDLE ); }
        void setDebounceTime(unsigned int debounce) { /* nothing */ }
        void setHoldTime(unsigned int hold) { /* nothing */ }
        void addEventListener(void (*listener)(char)) { /* nothing */ }

private:
        void transitionTo(KeypadState newState) { /* nothing */ }
        void initializePins() { /* nothing */ }
};

