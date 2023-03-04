#ifndef _RND
#define _RND

#ifdef __cplusplus
extern "C"{
#endif


#define RNG_M 2147483647L	/* m = 2^31 - 1 */
#define RNG_A 48271L
#define RNG_Q 127773L		/* m div a */
#define RNG_R 2836L			/* m mod a */

extern int32_t	get_rnd_seed(void);
extern void	reset_rnd_seed(void);
extern void	set_rnd_seed(int32_t seedval);
extern int32_t	rnd(void);
extern int32_t	xrnd(int32_t maxrnd);
extern bool chance(int percentage);

#ifdef __cplusplus
};
#endif
#endif
