<?php
namespace ION;

use ION\Process;
use ION\Process\Signal as Sig;
use ION\TestCase;

/**
 * Test ION\Process class
 * @see stubs/ION/Process.php
 */
class ProcessTest extends TestCase {

    /**
     * @memcheck
     */
    public function testPids() {
        $this->assertSame(getmypid(), Process::getPid());
        $this->assertSame(posix_getppid(), Process::getParentPid());
    }
    
    /**
     * @mem-check
     * @todo
     * @group testExecEmpty
     */
    public function _testExecEmpty() {
        Process::exec("sleep 0.2; echo 'done'");
        \ION::stop(.03);
        \ION::dispatch();
    }
    
    /**
     * @point done
     * @mem-check 
     * @todo
     * @group testExec
     */
    public function _testExec() {
        $defer = Process::exec("sleep 0.2; echo 'done'");
        $this->assertInstanceOf('Defer', $defer);
        $defer->onResult(function ($data, $error, $arg) {
            $this->point('done', 0.2);
            $this->assertSame("done\n" ,$data['stdout']);
            $this->assertSame("" ,$data['stderr']);
            $this->assertSame(0 ,$data['status']);
            $this->assertSame(0 ,$data['killed']);
//            \ION::stop();
        });
        \ION::dispatch();
    }
    
    /**
     * @group testGetUser
     * @memcheck
     */
    public function testGetUser() {
	    $actual = Process::getUser();
	    $expected = posix_getpwnam(posix_getlogin());
	    $this->assertSame($expected['name'], $actual['name']);
	    $this->assertSame($expected['uid'], $actual['uid']);
	    $this->assertSame($expected['gid'], $actual['gid']);
	    $this->assertSame($expected['dir'], $actual['home']);
	    $this->assertSame($expected['shell'], $actual['shell']);
    }
    
    /**
     * @group testGetAnotherUser
     * @memcheck
     */
    public function testGetAnotherUser() {
        $actual = Process::getUser('nobody');
	    $expected = posix_getpwnam('nobody');
	    $this->assertEquals($expected['name'], $actual['name']);
	    $this->assertEquals($expected['gecos'], $actual['gecos']);
	    $this->assertSame($expected['uid'], $actual['uid']);
	    $this->assertSame($expected['gid'], $actual['gid']);
        $this->assertSame($expected['dir'], $actual['home']);
        $this->assertSame($expected['shell'], $actual['shell']);
    }
    
    /**
     * @group testGetAnotherUserUID
     * @memcheck
     */
    public function testGetAnotherUserUID() {
	    $actual = Process::getUser(intval(`id -u nobody`));
	    $expected = posix_getpwnam('nobody');
	    $this->assertSame($expected['name'], $actual['name']);
	    $this->assertSame($expected['uid'], $actual['uid']);
	    $this->assertSame($expected['gid'], $actual['gid']);
	    $this->assertSame($expected['dir'], $actual['home']);
	    $this->assertSame($expected['shell'], $actual['shell']);
    }
    
    /**
     * @group testFork
     * @memcheck
     */
    public function _testFork() {
        $pid = Process::fork();
        if($pid) {
            $this->assertSame($pid, pcntl_waitpid($pid, $status));
            $this->assertSame(0, $status);
        } else {
            usleep(10000);
            exit(0);
        }
    }
    
    /**
     * @memcheck
     */
    public function testGetPriority() {
        $this->assertSame(pcntl_getpriority(), Process::getPriority());
        $this->assertSame(pcntl_getpriority(posix_getppid()), Process::getPriority(posix_getppid()));
    }
   
    
    /**
     * @group testSetPriority
     * @memcheck
     */
    public function testSetPriority() {
        $prio = Process::getPriority();
        $pid = Process::fork();
        
        if($pid) {
            usleep(10000);
            $this->assertSame($prio + 10, Process::getPriority($pid));
            $this->assertWaitPID($pid);
        } else {
            Process::setPriority(10);
            usleep(20000);
            exit(0);
        }
    }
    
    /**
     * @group testSetUser
     * @mem check
     */
    public function _testSetUser() {
        Process::setUser('_www', '_www');
        var_dump(Process::getUser());
        Process::setUser($_SERVER['USER'], 'staff');
    }
    
    /**
     * @group testOnSignal
     */
    public function _testOnSignal() {
        $this->out("test ".getmypid());
        $this->separate([$this, 'procOnSignal']);
        Process::onSignal(Sig::USR1, function($signal, $arg) {
            $this->assertSame(SIGUSR1, $signal);
            $this->assertSame('zx', $arg);
            $this->point('signal');
            \ION::stop();
        }, 'zx');
        \ION::dispatch();
    }
    
    public function procOnSignal() {
        $this->out("child ".getmypid());
        posix_kill(posix_getppid(), SIGUSR1);
    }
    
}