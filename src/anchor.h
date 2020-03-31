#ifndef COH_ANCHOR_H
#define COH_ANCHOR_H

/*
 * anchor.h
 * - Used for anchoring components of windows to a corner, etc.
 */

// Shift x bits.
#define COH_BITS(x) (1 << x)

// Anchor shorthands.
#define ANCH_TL    (anchor)(anchor::TOP    | anchor::LEFT  )
#define ANCH_TR    (anchor)(anchor::TOP    | anchor::RIGHT )
#define ANCH_BL    (anchor)(anchor::BOTTOM | anchor::LEFT  )
#define ANCH_BR    (anchor)(anchor::BOTTOM | anchor::RIGHT )
#define ANCH_CTR   (anchor)(anchor::CENT_V | anchor::CENT_H)
#define ANCH_CTR_L (anchor)(anchor::CENT_V | anchor::LEFT  )
#define ANCH_CTR_R (anchor)(anchor::CENT_V | anchor::RIGHT )
#define ANCH_CTR_T (anchor)(anchor::TOP    | anchor::CENT_H)
#define ANCH_CTR_B (anchor)(anchor::BOTTOM | anchor::CENT_H)

enum anchor : unsigned char
{
	LEFT   = COH_BITS(0),
	RIGHT  = COH_BITS(1),
	TOP    = COH_BITS(2),
	BOTTOM = COH_BITS(3),
	CENT_H = COH_BITS(4),
	CENT_V = COH_BITS(5)
};

#endif
