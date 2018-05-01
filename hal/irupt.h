#ifndef IRUPT_H_
#define IRUPT_H_

  //! @name Interrupt Handlers
  //! These are the interrupt handlers used by the \ref comm Module.
  //! @{
  __interrupt void vPORT1_ISR(void);
  __interrupt void PORT2_ISR(void);
  __interrupt void TIMERA0_ISR(void);
  __interrupt void vTIMERA1_ISR(void);
  __interrupt void TIMER0_B0_ISR(void);
  
  //! @}
  
#endif /*IRUPT_H_*/
