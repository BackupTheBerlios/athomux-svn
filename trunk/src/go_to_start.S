//////////////////////////////////////////////////////////////////////////////////
//
//  go_to_start.S 
//
//  Copyright (C) 2004 by Jonas Maebe, Ghent University, Belgium
//
//  This file is part of DIOTA (http://www.elis.UGent.be/diota/)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
//
//////////////////////////////////////////////////////////////////////////////////
//
// $Author: nieblifn $
// $Date: 2004/08/21 12:33:57 $
// $Log: go_to_start.S,v $
// Revision 1.1  2004/08/21 12:33:57  nieblifn
// *** empty log message ***
//
// Revision 1.1  2004/05/24 13:43:31  nieblifn
// *** empty log message ***
//
// Revision 1.2  2004/05/10 14:22:56  nieblifn
// clone & exec funktioniert mit execimage, test/fs/fs, und busybox.uclibc
//
// Revision 1.1  2004/04/30 12:06:59  nieblifn
// *** empty log message ***
//
// Revision 1.2  2004/04/14 12:30:11  nieblifn
// *** empty log message ***
//
//
// Revision 1.1.2.3  2004/01/22 13:18:10  jmaebe
// * changed copy loops to "rep movsl" so there are no separate basic blocks
//   anymore
// - removed "xorl %ecx,%ecx", as it's already zero after the copy loop
// + added zeroing of esi and edi
//
// Revision 1.1.2.2  2004/01/22 12:56:07  ronsse
// *** empty log message ***
//
// Revision 1.1.2.1  2004/01/20 13:21:56  ronsse
// + Support for statically linked binaries.
//
//
//////////////////////////////////////////////////////////////////////////////////

.globl go_to_start
  .type go_to_start,@function

go_to_start:
   pushl %ebp
   movl  %esp, %ebp
   // 4(ebp)  : ret address to main
   // 8(ebp)  : argc (int)
   // 12(ebp) : argv (char**)
   // 16(ebp) : envc (int)
   // 20(ebp) : envp (char**)
   // 24(ebp) : address of _start
   movl  8(%ebp),%eax
   movl  %eax,%ecx
   addl  16(%ebp),%eax
   addl  $3,%eax             // reserve room for argc + all argvs + NULL + envs + NULL
   shll  $2,%eax
   subl  %eax, %esp
   movl  %ecx, (%esp)        // store argc in place
   inc   %ecx                // make sure we also store the terminating NULL pointer of argv
   leal  4(%esp),%edi        // now store all argvs
   movl  12(%ebp),%esi
   rep   movsl
   movl  16(%ebp),%ecx       // now store all envps + terminating NULL
   incl  %ecx
   movl  20(%ebp),%esi
   rep   movsl
   xorl  %eax,%eax
   xorl  %ebx,%ebx
   xorl  %edx,%edx
   xorl  %esi,%esi
   xorl  %edi,%edi
   jmpl  *24(%ebp)
   // should never be reached
   int3

  .size go_to_start,.-go_to_start

