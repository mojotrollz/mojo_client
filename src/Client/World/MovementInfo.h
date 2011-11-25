#ifndef _MOVEMENTINFO_H
#define _MOVEMENTINFO_H

enum MovementFlags
{
    MOVEMENTFLAG_NONE               = 0x00000000,
    MOVEMENTFLAG_FORWARD            = 0x00000001,
    MOVEMENTFLAG_BACKWARD           = 0x00000002,
    MOVEMENTFLAG_STRAFE_LEFT        = 0x00000004,
    MOVEMENTFLAG_STRAFE_RIGHT       = 0x00000008,
    MOVEMENTFLAG_TURN_LEFT          = 0x00000010,
    MOVEMENTFLAG_TURN_RIGHT         = 0x00000020,
    MOVEMENTFLAG_PITCH_UP           = 0x00000040,
    MOVEMENTFLAG_PITCH_DOWN         = 0x00000080,
    MOVEMENTFLAG_WALK_MODE          = 0x00000100,               // Walking
    MOVEMENTFLAG_ONTRANSPORT        = 0x00000200,
    MOVEMENTFLAG_LEVITATING         = 0x00000400,
    MOVEMENTFLAG_ROOT               = 0x00000800,
    MOVEMENTFLAG_FALLING            = 0x00001000,
    MOVEMENTFLAG_FALLINGFAR         = 0x00002000,
    MOVEMENTFLAG_PENDINGSTOP        = 0x00004000,
    MOVEMENTFLAG_PENDINGSTRAFESTOP  = 0x00008000,
    MOVEMENTFLAG_PENDINGFORWARD     = 0x00010000,
    MOVEMENTFLAG_PENDINGBACKWARD    = 0x00020000,
    MOVEMENTFLAG_PENDINGSTRAFELEFT  = 0x00040000,
    MOVEMENTFLAG_PENDINGSTRAFERIGHT = 0x00080000,
    MOVEMENTFLAG_PENDINGROOT        = 0x00100000,
    MOVEMENTFLAG_SWIMMING           = 0x00200000,               // appears with fly flag also
    MOVEMENTFLAG_ASCENDING          = 0x00400000,               // swim up also
    MOVEMENTFLAG_DESCENDING         = 0x00800000,               // swim down also
    MOVEMENTFLAG_CAN_FLY            = 0x01000000,               // can fly in 3.3?
    MOVEMENTFLAG_FLYING             = 0x02000000,               // Actual flying mode
    MOVEMENTFLAG_SPLINE_ELEVATION   = 0x04000000,               // used for flight paths
    MOVEMENTFLAG_SPLINE_ENABLED     = 0x08000000,               // used for flight paths
    MOVEMENTFLAG_WATERWALKING       = 0x10000000,               // prevent unit from falling through water
    MOVEMENTFLAG_SAFE_FALL          = 0x20000000,               // active rogue safe fall spell (passive)
    MOVEMENTFLAG_HOVER              = 0x40000000
};

enum MovementFlags2
{
    MOVEMENTFLAG2_NONE              = 0x0000,
    MOVEMENTFLAG2_UNK1              = 0x0001,
    MOVEMENTFLAG2_UNK2              = 0x0002,
    MOVEMENTFLAG2_UNK3              = 0x0004,
    MOVEMENTFLAG2_FULLSPEEDTURNING  = 0x0008,
    MOVEMENTFLAG2_FULLSPEEDPITCHING = 0x0010,
    MOVEMENTFLAG2_ALLOW_PITCHING    = 0x0020,
    MOVEMENTFLAG2_UNK4              = 0x0040,
    MOVEMENTFLAG2_UNK5              = 0x0080,
    MOVEMENTFLAG2_UNK6              = 0x0100,
    MOVEMENTFLAG2_UNK7              = 0x0200,
    MOVEMENTFLAG2_INTERP_MOVEMENT   = 0x0400,
    MOVEMENTFLAG2_INTERP_TURNING    = 0x0800,
    MOVEMENTFLAG2_INTERP_PITCHING   = 0x1000,
    MOVEMENTFLAG2_UNK8              = 0x2000,
    MOVEMENTFLAG2_UNK9              = 0x4000,
    MOVEMENTFLAG2_UNK10             = 0x8000,
    MOVEMENTFLAG2_INTERP_MASK       = MOVEMENTFLAG2_INTERP_MOVEMENT | MOVEMENTFLAG2_INTERP_TURNING | MOVEMENTFLAG2_INTERP_PITCHING
};

enum SplineFlags{
    SF_None         = 0x00000000,
                                          // x00-xFF(first byte) used as animation Ids storage in pair with Animation flag
    SF_Done         = 0x00000100,
    SF_Falling      = 0x00000200,           // Affects elevation computation, can't be combined with Parabolic flag
    SF_No_Spline    = 0x00000400,
    SF_Parabolic    = 0x00000800,           // Affects elevation computation, can't be combined with Falling flag
    SF_Walkmode     = 0x00001000,
    SF_Flying       = 0x00002000,           // Smooth movement(Catmullrom interpolation mode), flying animation
    SF_OrientationFixed = 0x00004000,       // Model orientation fixed
    SF_Final_Point  = 0x00008000,
    SF_Final_Target = 0x00010000,
    SF_Final_Angle  = 0x00020000,
    SF_Catmullrom   = 0x00040000,           // Used Catmullrom interpolation mode
    SF_Cyclic       = 0x00080000,           // Movement by cycled spline
    SF_Enter_Cycle  = 0x00100000,           // Everytimes appears with cyclic flag in monster move packet, erases first spline vertex after first cycle done
    SF_Animation    = 0x00200000,           // Plays animation after some time passed
    SF_Frozen       = 0x00400000,           // Will never arrive
    SF_Unknown5     = 0x00800000,
    SF_Unknown6     = 0x01000000,
    SF_Unknown7     = 0x02000000,
    SF_Unknown8     = 0x04000000,
    SF_OrientationInversed = 0x08000000,
    SF_Unknown10    = 0x10000000,
    SF_Unknown11    = 0x20000000,
    SF_Unknown12    = 0x40000000,
    SF_Unknown13    = 0x80000000,
};


struct MovementInfo
{
    static uint8 _c; //Version switch helper
    
    // Read/Write methods
    void Read(ByteBuffer &data);
    void Write(ByteBuffer &data) const;
    
    // common
    uint32  flags;
    uint16  flags2;
    uint32  time;
    WorldPosition pos;
    // transport
    uint64  t_guid;
    WorldPosition   t_pos;
    uint32  t_time, t_time2;
    uint8    t_seat;
    // swimming and unk
    float   s_angle;
    // last fall time
    uint32  fallTime;
    // jumping
    float   j_velocity, j_sinAngle, j_cosAngle, j_xyspeed;
    // spline
    float   u_unk1;

    MovementInfo()
    {
        flags = time = t_time = fallTime = flags2 = 0;
        t_seat = 0;
        s_angle = j_velocity = j_sinAngle = j_cosAngle = j_xyspeed = u_unk1 = 0.0f;
        t_guid = 0;
    }

    void SetMovementFlags(uint32 _flags)
    {
        flags = _flags;
    }
};

inline ByteBuffer& operator<< (ByteBuffer& buf, MovementInfo const& mi)
{
    mi.Write(buf);
    return buf;
}

inline ByteBuffer& operator>> (ByteBuffer& buf, MovementInfo& mi)
{
    mi.Read(buf);
    return buf;
}


#endif
