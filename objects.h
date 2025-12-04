
enum DrawPriority {
    DRP_Neutral,
    DRP_Low,
    DRP_High
};

// Holds Position, Rotation and Transform for an "object". Probably could use a better name.
typedef struct GameObject {
    VECTOR position;
    SVECTOR rotation;
    MATRIX transform;
} GameObject;

// Same as GameObject, except uses a VECTOR for rotation instead of SVECTOR
typedef struct CameraObject {
    VECTOR position;
    VECTOR rotation;
    MATRIX transform;
} CameraObject;

// Extends GameObject and can also hold all the data needed to draw a polygon
typedef struct PolyObject {
    GameObject obj;
    MATRIX renderTransform;

    u_char polySides;
    ushort polyLength;
    void* polyPtr;
    SVECTOR* verticesPtr;
    int* indicesPtr;
    enum DrawPriority drPrio;

    int boxHeight;
    int boxWidth;

    bool collides;

    //void (*add)(struct PolyObject* self, u_long* ot);
} PolyObject;

typedef struct PlayerObject {
    PolyObject poly;
    CameraObject* cameraPtr;
    VECTOR velocity;
} PlayerObject;
